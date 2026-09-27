/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "AxisPointsValidator.h"
#include "Point.h"
#include <QObject>
#include <qmath.h>

namespace {

// Two graph coordinates are the same when they match to this relative tolerance, which is far below
// anything a digitizer can resolve but above double rounding noise
const double DUPLICATE_RELATIVE_TOLERANCE = 1e-6;

bool coordinatesMatch (double a,
                       double b)
{
  const double tolerance = DUPLICATE_RELATIVE_TOLERANCE * qMax (1.0, qMax (qAbs (a), qAbs (b)));

  return qAbs (a - b) <= tolerance;
}

QString numberToString (double value)
{
  return QString::number (value, 'g', 12);
}

}

AxisPointsValidator::AxisPointsValidator (const QList<Point> &points)
{
  checkDuplicates (points);
  checkXOrder (points);
  checkYOrder (points);
}

const QStringList &AxisPointsValidator::findings () const
{
  return m_findings;
}

void AxisPointsValidator::checkDuplicates (const QList<Point> &points)
{
  // Only complete (x and y) axis points can be compared as coordinates
  for (int firstIndex = 0; firstIndex < points.count (); firstIndex++) {

    const Point &pointFirst = points.at (firstIndex);
    if (pointFirst.isXOnly () || !pointFirst.hasPosGraph ()) {
      continue;
    }

    for (int secondIndex = firstIndex + 1; secondIndex < points.count (); secondIndex++) {

      const Point &pointSecond = points.at (secondIndex);
      if (pointSecond.isXOnly () || !pointSecond.hasPosGraph ()) {
        continue;
      }

      if (coordinatesMatch (pointFirst.posGraph ().x (), pointSecond.posGraph ().x ()) &&
          coordinatesMatch (pointFirst.posGraph ().y (), pointSecond.posGraph ().y ())) {

        m_findings << QObject::tr ("%1 and %2 have the same graph coordinates (%3, %4), which the "
                                   "coordinate system cannot be defined from.")
                          .arg (pointFirst.identifier ())
                          .arg (pointSecond.identifier ())
                          .arg (numberToString (pointFirst.posGraph ().x ()))
                          .arg (numberToString (pointFirst.posGraph ().y ()));
      }
    }
  }
}

void AxisPointsValidator::checkXOrder (const QList<Point> &points)
{
  // X-only axis points define the x axis, so their x values are expected to grow with their ordinal
  double xPrevious = 0.0;
  bool havePrevious = false;

  for (int index = 0; index < points.count (); index++) {

    const Point &point = points.at (index);
    if (!point.isXOnly () || !point.hasPosGraph ()) {
      continue;
    }

    const double xCurrent = point.posGraph ().x ();

    if (havePrevious && (xCurrent < xPrevious)) {
      m_findings << QObject::tr ("X axis point %1 goes backwards: its x value %2 is smaller than the "
                                 "previous x value %3. The points may have been digitized in the wrong order.")
                        .arg (point.identifier ())
                        .arg (numberToString (xCurrent))
                        .arg (numberToString (xPrevious));
    }

    xPrevious = xCurrent;
    havePrevious = true;
  }
}

void AxisPointsValidator::checkYOrder (const QList<Point> &points)
{
  // Y-only axis points define the y axis, so their y values are expected to grow with their ordinal
  double yPrevious = 0.0;
  bool havePrevious = false;

  for (int index = 0; index < points.count (); index++) {

    const Point &point = points.at (index);
    if (point.isXOnly () || !point.hasPosGraph ()) {
      continue;
    }

    const double yCurrent = point.posGraph ().y ();

    if (havePrevious && (yCurrent < yPrevious)) {
      m_findings << QObject::tr ("Y axis point %1 goes backwards: its y value %2 is smaller than the "
                                 "previous y value %3. The points may have been digitized in the wrong order.")
                        .arg (point.identifier ())
                        .arg (numberToString (yCurrent))
                        .arg (numberToString (yPrevious));
    }

    yPrevious = yCurrent;
    havePrevious = true;
  }
}
