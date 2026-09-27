/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "QualityReport.h"
#include "CallbackSearchReturn.h"
#include "Curve.h"
#include "CurvesGraphs.h"
#include "Document.h"
#include "Logger.h"
#include "Point.h"
#include "Transformation.h"
#include "functor.h"
#include <QMap>
#include <QObject>
#include <QTextStream>
#include <algorithm>
#include <cmath>

namespace {

// A repeated x value is only a candidate when the values really match, not when they merely round
// to the same printed value
const double X_RELATIVE_TOLERANCE = 1e-9;

// How far a slope change has to stand out from the typical change of the curve, in robust
// standard deviations (median absolute deviation), before the point in between is listed. A
// mean/standard deviation test cannot be used here: a single kink inflates the deviation so much
// that it hides itself.
const double STEEP_CHANGE_ROBUST_Z_SCORE = 3.5;

// Screen distance in pixels below which two points of different curves are considered to overlap
const double OVERLAP_SCREEN_DISTANCE = 2.0;

// Overlap reporting is quadratic, so very large curves are skipped instead of freezing the report
const int OVERLAP_MAX_POINTS_PER_CURVE = 2000;

/// Collect the points of every curve, grouped by curve name
class CollectPointsByCurve
{
public:
  CollectPointsByCurve (QMap<QString, QList<QualityPoint> > &pointsByCurve) :
    m_pointsByCurve (pointsByCurve)
  {
  }

  CallbackSearchReturn callback (const QString &curveName,
                                 const Point &point)
  {
    QualityPoint qualityPoint;
    qualityPoint.identifier = point.identifier ();
    qualityPoint.posScreen = point.posScreen ();
    qualityPoint.posGraph = point.hasPosGraph () ? point.posGraph () : QPointF ();

    m_pointsByCurve [curveName] << qualityPoint;

    return CALLBACK_SEARCH_RETURN_CONTINUE;
  }

private:
  CollectPointsByCurve ();

  QMap<QString, QList<QualityPoint> > &m_pointsByCurve;
};

bool pointsRepeated (double xBefore,
                     double xAfter)
{
  const double tolerance = X_RELATIVE_TOLERANCE * std::max (1.0, std::max (std::fabs (xBefore), std::fabs (xAfter)));

  return std::fabs (xAfter - xBefore) <= tolerance;
}

QString numberToString (double value)
{
  return QString::number (value, 'g', 12);
}

}

QualityReport::QualityReport ()
{
}

const QList<QualityIssue> &QualityReport::issues () const
{
  return m_issues;
}

QString QualityReport::issueTypeName (QualityIssueType type)
{
  switch (type) {
  case QUALITY_ISSUE_X_REPEATED:
    return QObject::tr ("Repeated x value");

  case QUALITY_ISSUE_X_REVERSED:
    return QObject::tr ("x value goes backwards");

  case QUALITY_ISSUE_STEEP_CHANGE:
    return QObject::tr ("Sudden slope change");

  case QUALITY_ISSUE_CURVE_OVERLAP:
    return QObject::tr ("Overlaps another curve");

  case QUALITY_ISSUE_OUTSIDE_AXES:
    return QObject::tr ("Outside the axes range");

  default:
    return QObject::tr ("Unknown");
  }
}

QString QualityReport::summary () const
{
  if (m_issues.isEmpty ()) {
    return QObject::tr ("No review candidates found.");
  }

  return QObject::tr ("%1 review candidates found.").arg (m_issues.count ());
}

QString QualityReport::toCsv () const
{
  QString csv;

  QTextStream str (&csv);
  str << "curve,point,issue,x,y,detail\n";

  for (int index = 0; index < m_issues.count (); index++) {
    const QualityIssue &issue = m_issues.at (index);

    str << issue.curveName << ","
        << issue.pointIdentifier << ","
        << issueTypeName (issue.type) << ","
        << numberToString (issue.posGraph.x ()) << ","
        << numberToString (issue.posGraph.y ()) << ","
        << issue.detail << "\n";
  }

  return csv;
}

void QualityReport::addIssue (const QString &curveName,
                              const QString &pointIdentifier,
                              const QPointF &posGraph,
                              QualityIssueType type,
                              const QString &detail)
{
  QualityIssue issue;
  issue.curveName = curveName;
  issue.pointIdentifier = pointIdentifier;
  issue.posGraph = posGraph;
  issue.type = type;
  issue.detail = detail;

  m_issues << issue;
}

void QualityReport::checkCurvePoints (const QString &curveName,
                                      const QList<QualityPoint> &points)
{
  // 1) x values that repeat or go backwards: the curve doubles back on itself, which is either
  // intentional (a relation) or a sign that points were ordered or matched incorrectly
  for (int index = 1; index < points.count (); index++) {

    const double xBefore = points.at (index - 1).posGraph.x ();
    const double xAfter = points.at (index).posGraph.x ();
    const QString identifier = points.at (index).identifier;
    const QPointF posGraph = points.at (index).posGraph;

    if (pointsRepeated (xBefore, xAfter)) {
      addIssue (curveName,
                identifier,
                posGraph,
                QUALITY_ISSUE_X_REPEATED,
                QString ("x=%1 repeats the previous point").arg (numberToString (xAfter)));
    } else if (xAfter < xBefore) {
      addIssue (curveName,
                identifier,
                posGraph,
                QUALITY_ISSUE_X_REVERSED,
                QString ("x went from %1 back to %2").arg (numberToString (xBefore),
                                                           numberToString (xAfter)));
    }
  }

  // 2) slope changes that stand out from the rest of the curve
  QList<double> slopeChanges;
  for (int index = 1; index + 1 < points.count (); index++) {

    const QPointF before = points.at (index - 1).posGraph;
    const QPointF current = points.at (index).posGraph;
    const QPointF after = points.at (index + 1).posGraph;

    const double dxBefore = current.x () - before.x ();
    const double dxAfter = after.x () - current.x ();
    if ((dxBefore == 0.0) || (dxAfter == 0.0)) {
      slopeChanges << 0.0;
      continue;
    }

    const double slopeBefore = (current.y () - before.y ()) / dxBefore;
    const double slopeAfter = (after.y () - current.y ()) / dxAfter;

    slopeChanges << std::fabs (slopeAfter - slopeBefore);
  }

  const int slopeChangeCount = slopeChanges.count ();
  if (slopeChangeCount > 1) {

    QList<double> sorted = slopeChanges;
    std::sort (sorted.begin (), sorted.end ());
    const double median = sorted.at (slopeChangeCount / 2);

    // Median absolute deviation: a robust stand-in for the standard deviation
    QList<double> deviations;
    for (int index = 0; index < slopeChangeCount; index++) {
      deviations << std::fabs (slopeChanges.at (index) - median);
    }
    std::sort (deviations.begin (), deviations.end ());
    const double medianAbsoluteDeviation = deviations.at (slopeChangeCount / 2);

    for (int index = 0; index < slopeChangeCount; index++) {

      const double change = slopeChanges.at (index);
      const double difference = std::fabs (change - median);

      bool isCandidate = false;
      QString detail;

      if (medianAbsoluteDeviation > 0.0) {

        const double robustZScore = 0.6745 * difference / medianAbsoluteDeviation;
        if (robustZScore > STEEP_CHANGE_ROBUST_Z_SCORE) {
          isCandidate = true;
          detail = QString ("slope change %1 is %2 robust standard deviations from the typical change %3")
                          .arg (numberToString (change))
                          .arg (numberToString (robustZScore))
                          .arg (numberToString (median));
        }

      } else if (difference > 1e-9) {

        // Every other change is identical (a perfectly smooth or straight curve), so any change
        // that differs at all stands out
        isCandidate = true;
        detail = QString ("slope change %1 differs from the otherwise constant change %2")
                        .arg (numberToString (change))
                        .arg (numberToString (median));
      }

      if (isCandidate) {

        // The middle point of the triple is the one sitting at the kink
        const QualityPoint &point = points.at (index + 1);
        addIssue (curveName,
                  point.identifier,
                  point.posGraph,
                  QUALITY_ISSUE_STEEP_CHANGE,
                  detail);
      }
    }
  }
}

void QualityReport::checkOutsideAxes (const QString &curveName,
                                      const QList<QualityPoint> &points,
                                      double axesLeft,
                                      double axesRight,
                                      double axesTop,
                                      double axesBottom,
                                      bool hasAxesRange)
{
  if (!hasAxesRange) {
    return;
  }


  for (int index = 0; index < points.count (); index++) {

    const QPointF posGraph = points.at (index).posGraph;
    if ((posGraph.x () < axesLeft) ||
        (posGraph.x () > axesRight) ||
        (posGraph.y () < axesTop) ||
        (posGraph.y () > axesBottom)) {
      addIssue (curveName,
                points.at (index).identifier,
                posGraph,
                QUALITY_ISSUE_OUTSIDE_AXES,
                QString ("%1,%2 is outside %3,%4 to %5,%6")
                        .arg (numberToString (posGraph.x ()))
                        .arg (numberToString (posGraph.y ()))
                        .arg (numberToString (axesLeft))
                        .arg (numberToString (axesTop))
                        .arg (numberToString (axesRight))
                        .arg (numberToString (axesBottom)));
    }
  }
}

void QualityReport::analyzeCurves (const QStringList &curveNames,
                                   const QList<QList<QualityPoint> > &curvesPoints,
                                   const QList<QualityPoint> &axisPoints)
{
  LOG4CPP_INFO_S ((*mainCat)) << "QualityReport::analyzeCurves curves=" << curveNames.count ();

  m_issues.clear ();

  // Range covered by the axis points, stored as plain doubles. The half unit margin on the right
  // and bottom counts a point that sits exactly on an axis coordinate as inside its own axes.
  // Plain comparisons are used instead of QRectF because the rectangle semantics (normalization,
  // half open edges, null handling) were producing candidates for every point.
  bool hasAxesRange = false;
  double axesLeft = 0.0;
  double axesRight = 0.0;
  double axesTop = 0.0;
  double axesBottom = 0.0;
  for (int index = 0; index < axisPoints.count (); index++) {

    const QPointF posGraph = axisPoints.at (index).posGraph;
    if (!hasAxesRange) {
      axesLeft = axesRight = posGraph.x ();
      axesTop = axesBottom = posGraph.y ();
      hasAxesRange = true;
    } else {
      axesLeft = std::min (axesLeft, posGraph.x ());
      axesRight = std::max (axesRight, posGraph.x ());
      axesTop = std::min (axesTop, posGraph.y ());
      axesBottom = std::max (axesBottom, posGraph.y ());
    }
  }

  if (hasAxesRange) {
    axesRight += 0.5;
    axesBottom += 0.5;
  }

  for (int curveIndex = 0; curveIndex < curvesPoints.count (); curveIndex++) {

    const QString curveName = curveIndex < curveNames.count () ? curveNames.at (curveIndex) : QString ();
    const QList<QualityPoint> &points = curvesPoints.at (curveIndex);

    checkCurvePoints (curveName, points);
    checkOutsideAxes (curveName,
                      points,
                      axesLeft,
                      axesRight,
                      axesTop,
                      axesBottom,
                      hasAxesRange);
  }

  // Points of different curves that sit on top of each other, which usually means one curve was
  // digitized twice
  for (int firstCurve = 0; firstCurve < curvesPoints.count (); firstCurve++) {
    for (int secondCurve = firstCurve + 1; secondCurve < curvesPoints.count (); secondCurve++) {

      const QList<QualityPoint> &pointsFirst = curvesPoints.at (firstCurve);
      const QList<QualityPoint> &pointsSecond = curvesPoints.at (secondCurve);

      if ((pointsFirst.count () > OVERLAP_MAX_POINTS_PER_CURVE) ||
          (pointsSecond.count () > OVERLAP_MAX_POINTS_PER_CURVE)) {
        LOG4CPP_INFO_S ((*mainCat)) << "QualityReport::analyzeCurves overlap check skipped for large curves";
        continue;
      }

      for (int firstIndex = 0; firstIndex < pointsFirst.count (); firstIndex++) {
        for (int secondIndex = 0; secondIndex < pointsSecond.count (); secondIndex++) {

          const QPointF screenFirst = pointsFirst.at (firstIndex).posScreen;
          const QPointF screenSecond = pointsSecond.at (secondIndex).posScreen;
          const double dx = screenFirst.x () - screenSecond.x ();
          const double dy = screenFirst.y () - screenSecond.y ();

          if (std::sqrt (dx * dx + dy * dy) <= OVERLAP_SCREEN_DISTANCE) {

            const QString firstName = firstCurve < curveNames.count () ? curveNames.at (firstCurve) : QString ();
            const QString secondName = secondCurve < curveNames.count () ? curveNames.at (secondCurve) : QString ();

            addIssue (firstName,
                      pointsFirst.at (firstIndex).identifier,
                      pointsFirst.at (firstIndex).posGraph,
                      QUALITY_ISSUE_CURVE_OVERLAP,
                      QString ("sits on a point of %1 within %2 pixels").arg (secondName)
                                                                     .arg (numberToString (OVERLAP_SCREEN_DISTANCE)));

            addIssue (secondName,
                      pointsSecond.at (secondIndex).identifier,
                      pointsSecond.at (secondIndex).posGraph,
                      QUALITY_ISSUE_CURVE_OVERLAP,
                      QString ("sits on a point of %1 within %2 pixels").arg (firstName)
                                                                     .arg (numberToString (OVERLAP_SCREEN_DISTANCE)));
          }
        }
      }
    }
  }

  LOG4CPP_INFO_S ((*mainCat)) << "QualityReport::analyzeCurves issues=" << m_issues.count ();
}

void QualityReport::analyze (const Document &document,
                             const Transformation &transformation)
{
  LOG4CPP_INFO_S ((*mainCat)) << "QualityReport::analyze";

  // Axis points, which define the graph range
  QList<QualityPoint> axisPoints;
  {
    QMap<QString, QList<QualityPoint> > axisPointsByCurve;
    CollectPointsByCurve collectAxisPoints (axisPointsByCurve);
    Functor2wRet<const QString &, const Point &, CallbackSearchReturn> ftorAxis = functor_ret (collectAxisPoints,
                                                                                               &CollectPointsByCurve::callback);
    document.curveAxes ().iterateThroughCurvePoints (ftorAxis);
    axisPoints = axisPointsByCurve.value (AXIS_CURVE_NAME);
  }

  // Graph points, curve by curve. The transformation is only needed for points whose graph
  // coordinates were never stored.
  const QStringList curveNames = document.curvesGraphsNames ();

  QMap<QString, QList<QualityPoint> > pointsByCurve;
  {
    CollectPointsByCurve collectPoints (pointsByCurve);
    Functor2wRet<const QString &, const Point &, CallbackSearchReturn> ftor = functor_ret (collectPoints,
                                                                                           &CollectPointsByCurve::callback);
    document.curvesGraphs ().iterateThroughCurvesPoints (ftor);
  }

  QStringList curveNamesWithPoints;
  QList<QList<QualityPoint> > curvesPoints;

  for (int curveIndex = 0; curveIndex < curveNames.count (); curveIndex++) {

    const QString curveName = curveNames.at (curveIndex);
    QList<QualityPoint> points = pointsByCurve.value (curveName);
    if (points.isEmpty ()) {
      continue;
    }

    // Fill in the graph coordinates of points that only have screen coordinates
    for (int index = 0; index < points.count (); index++) {
      if (points.at (index).posGraph.isNull ()) {
        QPointF posGraph;
        transformation.transformScreenToRawGraph (points.at (index).posScreen,
                                                  posGraph);
        points [index].posGraph = posGraph;
      }
    }

    curveNamesWithPoints << curveName;
    curvesPoints << points;
  }

  // Axis points may also be missing their graph coordinates
  for (int index = 0; index < axisPoints.count (); index++) {
    if (axisPoints.at (index).posGraph.isNull ()) {
      QPointF posGraph;
      transformation.transformScreenToRawGraph (axisPoints.at (index).posScreen,
                                                posGraph);
      axisPoints [index].posGraph = posGraph;
    }
  }

  analyzeCurves (curveNamesWithPoints,
                 curvesPoints,
                 axisPoints);
}
