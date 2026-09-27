/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#ifndef CALLBACK_AUTOMATED_POINTS_H
#define CALLBACK_AUTOMATED_POINTS_H

#include "CallbackSearchReturn.h"
#include <QStringList>

class Point;

/// Callback that collects the identifiers of every point that was placed by automation
/// (Segment Fill or Point Match), so those points can be deleted and re-digitized while
/// the points placed by hand are kept.
class CallbackAutomatedPoints
{
public:
  /// Single constructor. Identifiers are appended to the list that is passed in.
  CallbackAutomatedPoints (QStringList &pointIdentifiers);

  /// Callback method.
  CallbackSearchReturn callback (const QString &curveName,
                                 const Point &point);

private:
  CallbackAutomatedPoints ();

  QStringList &m_pointIdentifiers;
};

#endif // CALLBACK_AUTOMATED_POINTS_H
