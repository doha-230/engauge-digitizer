/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "CallbackAutomatedPoints.h"
#include "Point.h"
#include "PointOrigin.h"
#include "Logger.h"

CallbackAutomatedPoints::CallbackAutomatedPoints (QStringList &pointIdentifiers) :
  m_pointIdentifiers (pointIdentifiers)
{
}

CallbackSearchReturn CallbackAutomatedPoints::callback (const QString & /* curveName */,
                                                        const Point &point)
{
  if (pointOriginIsAutomated (point.origin ())) {
    m_pointIdentifiers << point.identifier ();
  }

  return CALLBACK_SEARCH_RETURN_CONTINUE;
}
