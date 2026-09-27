/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#ifndef SEGMENT_CENTER_STRATEGY_H
#define SEGMENT_CENTER_STRATEGY_H

#include <QString>

/// How the Auto Curve Detection and Segment Fill modes place a point vertically inside a thick curve
/// band:
/// - TRACED_PATH_CENTER keeps the upstream behavior: the point sits on the path the cursor or the
///   scan traced, rounded to a pixel. Default, so results are identical to the upstream release.
/// - PIXEL_RUN_CENTER moves each point to the center of the run of curve pixels in its column, which
///   follows the middle of a thick band.
/// - WEIGHTED_PIXEL_CENTER moves each point to the intensity weighted center of the run, which
///   follows the darkest middle of a band with soft edges.
enum SegmentCenterStrategy {
  TRACED_PATH_CENTER,
  PIXEL_RUN_CENTER,
  WEIGHTED_PIXEL_CENTER
};

/// Convert a strategy to the string stored in the settings
QString segmentCenterStrategyToString (SegmentCenterStrategy strategy);

/// Convert a settings string back to a strategy. Unknown names fall back to TRACED_PATH_CENTER.
SegmentCenterStrategy segmentCenterStrategyFromString (const QString &asString);

#endif // SEGMENT_CENTER_STRATEGY_H
