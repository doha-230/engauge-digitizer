/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#ifndef QUALITY_REPORT_H
#define QUALITY_REPORT_H

#include <QList>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <QStringList>

class Document;
class Transformation;

/// Kinds of points that deserve a second look. These are review candidates, not errors: for
/// example a curve that is digitized as a relation may legitimately visit the same x coordinate
/// more than once.
enum QualityIssueType {
  QUALITY_ISSUE_X_REPEATED = 0,
  QUALITY_ISSUE_X_REVERSED,
  QUALITY_ISSUE_STEEP_CHANGE,
  QUALITY_ISSUE_CURVE_OVERLAP,
  QUALITY_ISSUE_OUTSIDE_AXES
};

/// One point handed to the checks, with the coordinates the checks need. Keeping this separate from
/// Point keeps the checks free of the document and the coordinate transformation, which is also
/// what makes them directly testable.
struct QualityPoint {
  QString identifier;
  QPointF posScreen;
  QPointF posGraph;
};

/// One review candidate
struct QualityIssue {
  QString curveName;
  QString pointIdentifier;
  QPointF posGraph;
  QualityIssueType type;
  QString detail;
};

/// Inspects the digitized points of a document and lists the ones that deserve a second look, so
/// that a curve does not have to be checked by eye point by point. Typical use: run the report
/// after changing the color filter or the grid removal settings, or before exporting.
///
/// The checks are pure geometry: no file, image or network access.
class QualityReport
{
public:
  /// Single constructor
  QualityReport ();

  /// Check the points of one document. Any previous result is discarded.
  void analyze (const Document &document,
                const Transformation &transformation);

  /// Check points that were collected elsewhere. curveNames and curvesPoints are parallel lists,
  /// axisPoints holds the axis points of the document, and every list is expected to be ordered by
  /// the curve ordinal.
  void analyzeCurves (const QStringList &curveNames,
                      const QList<QList<QualityPoint> > &curvesPoints,
                      const QList<QualityPoint> &axisPoints);

  /// Review candidates found by the checks, in curve and point order
  const QList<QualityIssue> &issues () const;

  /// One line summary, suitable for a status bar or a dialog
  QString summary () const;

  /// Comma separated listing of every issue, with a header row
  QString toCsv () const;

  /// Human readable name of one issue type
  static QString issueTypeName (QualityIssueType type);

private:
  QualityReport (const QualityReport &other);

  /// Append one issue
  void addIssue (const QString &curveName,
                 const QString &pointIdentifier,
                 const QPointF &posGraph,
                 QualityIssueType type,
                 const QString &detail);

  /// x values that repeat or go backwards, plus slope changes that stand out
  void checkCurvePoints (const QString &curveName,
                         const QList<QualityPoint> &points);

  /// Points outside the range covered by the axis points
  void checkOutsideAxes (const QString &curveName,
                         const QList<QualityPoint> &points,
                         const QRectF &axesRange,
                         bool hasAxesRange);

  QList<QualityIssue> m_issues;
};

#endif // QUALITY_REPORT_H
