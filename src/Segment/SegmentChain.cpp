/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "SegmentChain.h"
#include "EngaugeAssert.h"
#include "Logger.h"
#include "Segment.h"
#include "SegmentLine.h"
#include <QLineF>
#include <QtAlgorithms>
#include <qmath.h>

SegmentChain::SegmentChain (const QList<SegmentChainLink> &links,
                            double maxGapPixels,
                            double maxTurnDegrees) :
  m_links (links),
  m_maxGapPixels (maxGapPixels),
  m_maxTurnDegrees (maxTurnDegrees),
  m_stoppedAtAngle (false),
  m_stoppedAtGap (false)
{
}

QPointF SegmentChain::startOf (Segment *segment)
{
  if (segment == nullptr || segment->lineCount () == 0) {
    return QPointF ();
  }

  return segment->firstPoint ();
}

QPointF SegmentChain::endOf (Segment *segment)
{
  if (segment == nullptr || segment->lineCount () == 0) {
    return QPointF ();
  }

  // The segment grows column by column, so its end is the end of the last line
  return segment->lastPoint ();
}

Segment *SegmentChain::neighborOf (Segment *segment,
                                   bool left) const
{
  // The neighbor shares a link with this segment and starts on the requested side
  const QPointF posSelf = startOf (segment);

  Segment *neighbor = nullptr;
  for (int index = 0; index < m_links.count (); index++) {
    const SegmentChainLink &link = m_links.at (index);

    if (link.segmentA != segment && link.segmentB != segment) {
      continue;
    }

    Segment *other = (link.segmentA == segment) ? link.segmentB : link.segmentA;
    const QPointF posOther = startOf (other);

    if (posOther.isNull () || posSelf.isNull ()) {
      continue;
    }

    if ((left && posOther.x () < posSelf.x ()) ||
        (!left && posOther.x () > posSelf.x ())) {

      if (neighbor == nullptr ||
          startOf (neighbor).isNull () ||
          qAbs (posOther.x () - posSelf.x ()) < qAbs (startOf (neighbor).x () - posSelf.x ())) {
        // Closest neighbor on this side wins
        neighbor = other;
      }
    }
  }

  return neighbor;
}

QList<Segment*> SegmentChain::chainFrom (Segment *segmentStart) const
{
  LOG4CPP_INFO_S ((*mainCat)) << "SegmentChain::chainFrom";

  m_stoppedAtAngle = false;
  m_stoppedAtGap = false;

  ENGAUGE_CHECK_PTR (segmentStart);

  QList<Segment*> chain;
  chain << segmentStart;

  // Walk to the left, then to the right. Segments are appended at the front or back accordingly
  for (int direction = 0; direction < 2; direction++) {
    const bool left = (direction == 0);

    Segment *current = segmentStart;
    while (true) {

      Segment *next = neighborOf (current, left);
      if (next == nullptr || chain.contains (next)) {
        break; // No neighbor, or already in the chain (closed loop safety)
      }

      // The gap between the touching ends decides whether the two pieces belong to the same curve
      const QPointF endCurrent = left ? startOf (current) : endOf (current);
      const QPointF startNext = left ? endOf (next) : startOf (next);

      if (endCurrent.isNull () || startNext.isNull ()) {
        break;
      }

      const double gap = qAbs (startNext.x () - endCurrent.x ());
      if (gap > m_maxGapPixels) {
        m_stoppedAtGap = true;
        LOG4CPP_INFO_S ((*mainCat)) << "SegmentChain::chainFrom stopped at gap"
                                    << " gap=" << gap;
        break;
      }

      // Direction continuity gate: a curve keeps its direction where two scanned pieces touch,
      // while a grid line crosses the curve at a large angle and a glyph turns randomly
      const QLineF lineEndCurrent = left ? QLineF (endOf (current), startOf (current)) :
                                     QLineF (startOf (current), endOf (current));
      const QLineF lineStartNext = left ? QLineF (startOf (next), endOf (next)) :
                                    QLineF (startOf (next), endOf (next));
      const double angleBetween = lineEndCurrent.angleTo (lineStartNext);
      if (angleBetween > m_maxTurnDegrees) {
        m_stoppedAtAngle = true;
        LOG4CPP_INFO_S ((*mainCat)) << "SegmentChain::chainFrom stopped at angle"
                                    << " angle=" << angleBetween;
        break;
      }

      if (left) {
        chain.prepend (next);
      } else {
        chain.append (next);
      }

      current = next;
    }
  }

  LOG4CPP_INFO_S ((*mainCat)) << "SegmentChain::chainFrom pieces=" << chain.count ()
                              << " stoppedAtGap=" << m_stoppedAtGap
                              << " stoppedAtAngle=" << m_stoppedAtAngle;

  return chain;
}

bool SegmentChain::stoppedAtAngle () const
{
  return m_stoppedAtAngle;
}

bool SegmentChain::stoppedAtGap () const
{
  return m_stoppedAtGap;
}
