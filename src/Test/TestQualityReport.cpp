/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "QualityReport.h"
#include <QtTest/QtTest>
#include <QStringList>
#include "Test/TestQualityReport.h"

namespace {

// Points are spaced far enough apart (about 14 pixels) that only points with the same coordinates
// count as overlapping
const double SPACING = 10.0;

QualityPoint makePoint (const QString &identifier,
                        double x,
                        double y)
{
  QualityPoint point;
  point.identifier = identifier;
  point.posScreen = QPointF (x, y); // Screen and graph coordinates coincide in these tests
  point.posGraph = QPointF (x, y);

  return point;
}

QList<QualityPoint> makeCurve (const QString &prefix,
                               const QList<double> &yValues)
{
  QList<QualityPoint> points;
  for (int index = 0; index < yValues.count (); index++) {
    points << makePoint (QString ("%1%2").arg (prefix).arg (index),
                         index * SPACING,
                         yValues.at (index));
  }

  return points;
}

QList<QualityPoint> makePointsFromCoordinates (const QString &prefix,
                                               const QList<QPointF> &coordinates)
{
  QList<QualityPoint> points;
  for (int index = 0; index < coordinates.count (); index++) {
    points << makePoint (QString ("%1%2").arg (prefix).arg (index),
                         coordinates.at (index).x (),
                         coordinates.at (index).y ());
  }

  return points;
}

// Straight rising line through the origin
QList<QualityPoint> makeStraightCurve (const QString &prefix)
{
  QList<double> yValues;
  for (int index = 0; index < 6; index++) {
    yValues << index * SPACING;
  }

  return makeCurve (prefix, yValues);
}

int countOfType (const QualityReport &report,
                 QualityIssueType type)
{
  int count = 0;
  const QList<QualityIssue> &issues = report.issues ();
  for (int index = 0; index < issues.count (); index++) {
    if (issues.at (index).type == type) {
      count++;
    }
  }

  return count;
}

}

QTEST_APPLESS_MAIN (TestQualityReport)

TestQualityReport::TestQualityReport(QObject *parent) :
  QObject(parent)
{
}

void TestQualityReport::initTestCase ()
{
}

void TestQualityReport::cleanupTestCase ()
{
}

void TestQualityReport::testCleanCurveHasNoIssues ()
{
  QStringList curveNames;
  QList<QList<QualityPoint> > curvesPoints;
  curveNames << "Curve1";
  curvesPoints << makeStraightCurve ("Clean");

  QList<QualityPoint> axisPoints;
  axisPoints << makePoint ("Axis0", 0, 0) << makePoint ("Axis1", 100, 100);

  QualityReport report;
  report.analyzeCurves (curveNames,
                        curvesPoints,
                        axisPoints);

  // A curve that is straight, inside the axes and alone must not produce a single candidate
  QCOMPARE (report.issues ().count (), 0);
  QVERIFY (report.summary ().contains ("No review"));
}

void TestQualityReport::testRepeatedAndReversedXAreReported ()
{
  QStringList curveNames;
  QList<QList<QualityPoint> > curvesPoints;
  QList<QPointF> coordinates;
  coordinates << QPointF (0, 0)
              << QPointF (10, 10)
              << QPointF (10, 20)  // x repeats the previous point
              << QPointF (30, 30)
              << QPointF (20, 40)  // x goes backwards
              << QPointF (50, 50);
  curveNames << "Curve1";
  curvesPoints << makePointsFromCoordinates ("Points", coordinates);

  QualityReport report;
  report.analyzeCurves (curveNames,
                        curvesPoints,
                        QList<QualityPoint> ());

  QCOMPARE (countOfType (report, QUALITY_ISSUE_X_REPEATED), 1);
  QCOMPARE (countOfType (report, QUALITY_ISSUE_X_REVERSED), 1);

  // The CSV listing carries a header row and the curve name, so a report can be attached to notes
  QVERIFY (report.toCsv ().startsWith ("curve,point,issue,x,y,detail"));
  QVERIFY (report.toCsv ().contains ("Curve1"));
}

void TestQualityReport::testSteepChangeIsReported ()
{
  QStringList curveNames;
  QList<QList<QualityPoint> > curvesPoints;
  QList<double> yValues;
  yValues << 0 << 10 << 20 << 30 << 40 << 400 << 60 << 70 << 80 << 90 << 100 << 110 << 120;
  curveNames << "Curve1";
  curvesPoints << makeCurve ("Points", yValues);

  QualityReport report;
  report.analyzeCurves (curveNames,
                        curvesPoints,
                        QList<QualityPoint> ());

  // A single spike on an otherwise straight curve is the case that a mean/standard deviation test
  // would miss, so it is the case this check exists for
  QVERIFY (countOfType (report, QUALITY_ISSUE_STEEP_CHANGE) >= 1);
}

void TestQualityReport::testOverlapBetweenCurvesIsReported ()
{
  QStringList curveNames;
  QList<QList<QualityPoint> > curvesPoints;
  curveNames << "Curve1" << "Curve2";
  curvesPoints << makeStraightCurve ("First") << makeStraightCurve ("Second");

  QualityReport report;
  report.analyzeCurves (curveNames,
                        curvesPoints,
                        QList<QualityPoint> ());

  // Two identical curves sit on top of each other, so each of the six coincidences is reported once
  // for each of the two curves
  QCOMPARE (countOfType (report, QUALITY_ISSUE_CURVE_OVERLAP), 12);
}

void TestQualityReport::testOutsideAxesIsReported ()
{
  QStringList curveNames;
  QList<QList<QualityPoint> > curvesPoints;
  QList<double> yValues;
  yValues << 0 << 10 << 990; // Last point is far outside the axes
  curveNames << "Curve1";
  curvesPoints << makeCurve ("Points", yValues);

  QList<QualityPoint> axisPoints;
  axisPoints << makePoint ("Axis0", 0, 0) << makePoint ("Axis1", 100, 100);

  QualityReport report;
  report.analyzeCurves (curveNames,
                        curvesPoints,
                        axisPoints);

  QCOMPARE (countOfType (report, QUALITY_ISSUE_OUTSIDE_AXES), 1);

  // Without axis points there is no range to compare against, so nothing is reported
  QualityReport reportWithoutAxes;
  reportWithoutAxes.analyzeCurves (curveNames,
                                   curvesPoints,
                                   QList<QualityPoint> ());
  QCOMPARE (countOfType (reportWithoutAxes, QUALITY_ISSUE_OUTSIDE_AXES), 0);
}
