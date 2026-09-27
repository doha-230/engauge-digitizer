/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#ifndef CALLBACK_COLLECT_AXIS_POINTS_H
#define CALLBACK_COLLECT_AXIS_POINTS_H

#include "CallbackSearchReturn.h"
#include <QList>

class Point;

/// Callback that collects the axis points into a list, in curve order, for reviews that want to look
/// at all of them at once.
class CallbackCollectAxisPoints
{
public:
  /// Single constructor. The points are appended to the list that is passed in.
  CallbackCollectAxisPoints (QList<Point> &points);

  /// Callback method.
  CallbackSearchReturn callback (const QString &curveName,
                                 const Point &point);

private:
  CallbackCollectAxisPoints ();

  QList<Point> &m_points;
};

#endif // CALLBACK_COLLECT_AXIS_POINTS_H
