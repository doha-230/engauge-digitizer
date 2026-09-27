/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "SegmentCenterStrategy.h"

QString segmentCenterStrategyToString (SegmentCenterStrategy strategy)
{
  switch (strategy) {
  case TRACED_PATH_CENTER:
    return "TracedPath";

  case PIXEL_RUN_CENTER:
    return "PixelRunCenter";

  case WEIGHTED_PIXEL_CENTER:
    return "WeightedPixelCenter";

  default:
    return "TracedPath";
  }
}

SegmentCenterStrategy segmentCenterStrategyFromString (const QString &asString)
{
  if (asString == "PixelRunCenter") {
    return PIXEL_RUN_CENTER;
  } else if (asString == "WeightedPixelCenter") {
    return WEIGHTED_PIXEL_CENTER;
  } else {
    // Unknown names keep the upstream behavior
    return TRACED_PATH_CENTER;
  }
}
