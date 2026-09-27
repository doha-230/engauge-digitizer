/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#ifndef AXIS_POINTS_VALIDATOR_H
#define AXIS_POINTS_VALIDATOR_H

#include <QList>
#include <QString>

class Point;

/// Reviews the axis points and lists what looks wrong, as a confirmation prompt rather than an
/// error. The three checks:
/// - two axis points with the same graph coordinates, which is almost always a repeated digitizing
///   mistake, since the transformation cannot be defined from two identical points;
/// - x values of the X-only axis points that go backwards, which suggests the points were digitized
///   in the wrong order;
/// - y values of the Y-only axis points that go downwards, which suggests the same.
///
/// This is pure logic over a point list, with no document or transformation, which is what makes it
/// directly testable.
class AxisPointsValidator
{
public:
  /// Single constructor. The points are the axis points of one document.
  AxisPointsValidator (const QList<Point> &points);

  /// Review findings, one line each. Empty when nothing looked wrong.
  const QStringList &findings () const;

private:
  AxisPointsValidator ();

  void checkDuplicates (const QList<Point> &points);
  void checkXOrder (const QList<Point> &points);
  void checkYOrder (const QList<Point> &points);

  QStringList m_findings;
};

#endif // AXIS_POINTS_VALIDATOR_H
