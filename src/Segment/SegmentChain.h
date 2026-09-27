/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#ifndef SEGMENT_CHAIN_H
#define SEGMENT_CHAIN_H

#include <QList>
#include <QPointF>

class Segment;

/// One link between two Segments that touch. The factory records these while it scans the image, so
/// no second scan is needed.
struct SegmentChainLink {
  Segment *segmentA;
  Segment *segmentB;
  double xContact; // Column where the two segments met
};

/// Chain of connected Segments, walked from the clicked Segment out to both ends.
///
/// This class assumes curves do not branch, which is the documented premise of the Auto Curve
/// Detection mode: each Segment then has at most one neighbor on each side, so the chain is walked
/// with a simple bidirectional loop instead of a graph search.
class SegmentChain
{
public:
  /// Single constructor. The links are the connections recorded by the segment factory. The maximum
  /// turn angle cuts the chain where two consecutive pieces meet at too sharp an angle, which is what
  /// stops the chain from following a grid line that crosses the curve.
  SegmentChain (const QList<SegmentChainLink> &links,
                double maxGapPixels,
                double maxTurnDegrees);

  /// Segments of the chain that contains the specified segment, ordered along the curve (from the
  /// left end to the right end as drawn). The clicked segment is part of the result. An empty list
  /// means the chain was cut by a direction change (the caller decides what to do).
  QList<Segment*> chainFrom (Segment *segmentStart) const;

  /// True when the walk stopped early because two consecutive segments meet at an angle above the
  /// maximum. Only valid after chainFrom returned a non-empty list.
  bool stoppedAtAngle () const;

  /// True when the walk stopped early because the next segment was further away than the maximum
  /// gap. Only valid after chainFrom returned a non-empty list.
  bool stoppedAtGap () const;

private:
  SegmentChain ();

  /// Neighbor of a segment on the left (lower firstPoint x) or right (higher firstPoint x), or null
  Segment *neighborOf (Segment *segment,
                       bool left) const;

  /// First point of a segment, or an invalid point for null. Static so it can be used on both sides
  /// of a link
  static QPointF startOf (Segment *segment);

  /// Last point of a segment (end of its last line), or an invalid point for null
  static QPointF endOf (Segment *segment);

  QList<SegmentChainLink> m_links;
  double m_maxGapPixels;
  double m_maxTurnDegrees;

  // The walk records why it stopped, so these are written by the const chainFrom
  mutable bool m_stoppedAtAngle;
  mutable bool m_stoppedAtGap;
};

#endif // SEGMENT_CHAIN_H
