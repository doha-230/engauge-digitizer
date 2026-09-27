/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "PointOrigin.h"

const QString POINT_ORIGIN_VALUE_UNKNOWN ("Unknown");
const QString POINT_ORIGIN_VALUE_MANUAL ("Manual");
const QString POINT_ORIGIN_VALUE_SEGMENT_FILL ("SegmentFill");
const QString POINT_ORIGIN_VALUE_POINT_MATCH ("PointMatch");
const QString POINT_ORIGIN_VALUE_PASTED ("Pasted");

QString pointOriginToString (PointOrigin origin)
{
  switch (origin) {
  case POINT_ORIGIN_MANUAL:
    return POINT_ORIGIN_VALUE_MANUAL;

  case POINT_ORIGIN_SEGMENT_FILL:
    return POINT_ORIGIN_VALUE_SEGMENT_FILL;

  case POINT_ORIGIN_POINT_MATCH:
    return POINT_ORIGIN_VALUE_POINT_MATCH;

  case POINT_ORIGIN_PASTED:
    return POINT_ORIGIN_VALUE_PASTED;

  case POINT_ORIGIN_UNKNOWN:
  default:
    return POINT_ORIGIN_VALUE_UNKNOWN;
  }
}

PointOrigin pointOriginFromString (const QString &name)
{
  if (name == POINT_ORIGIN_VALUE_MANUAL) {
    return POINT_ORIGIN_MANUAL;
  } else if (name == POINT_ORIGIN_VALUE_SEGMENT_FILL) {
    return POINT_ORIGIN_SEGMENT_FILL;
  } else if (name == POINT_ORIGIN_VALUE_POINT_MATCH) {
    return POINT_ORIGIN_POINT_MATCH;
  } else if (name == POINT_ORIGIN_VALUE_PASTED) {
    return POINT_ORIGIN_PASTED;
  } else {
    return POINT_ORIGIN_UNKNOWN;
  }
}

bool pointOriginIsAutomated (PointOrigin origin)
{
  return (origin == POINT_ORIGIN_SEGMENT_FILL) ||
         (origin == POINT_ORIGIN_POINT_MATCH);
}
