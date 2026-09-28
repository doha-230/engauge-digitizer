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
#include <QHash>
#include <QLineF>
#include <QtAlgorithms>
#include <qmath.h>
#include <queue>
#include <vector>

namespace {

struct PathCandidate {
  double cost;
  Segment *segment;
};

struct PathCandidateGreater {
  bool operator() (const PathCandidate &a, const PathCandidate &b) const
  {
    return a.cost > b.cost;
  }
};

}

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
  // A crossing may offer several links. Prefer the piece that continues in the same
  // direction, rather than the one whose start is nearest in x.
  const QPointF posSelf = startOf (segment);
  const QPointF endCurrent = left ? startOf (segment) : endOf (segment);
  const QLineF currentLine = left ? QLineF (endOf (segment), startOf (segment)) :
                                    QLineF (startOf (segment), endOf (segment));

  Segment *neighbor = nullptr;
  double bestAngle = 361.0;
  double bestGap = 1e100;
  bool rejectedAtGap = false;
  bool rejectedAtAngle = false;
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
      const QPointF startNext = left ? endOf (other) : startOf (other);
      if (startNext.isNull ()) {
        continue;
      }
      const double gap = QLineF (endCurrent, startNext).length ();
      if (gap > m_maxGapPixels) {
        rejectedAtGap = true;
        continue;
      }

      const QLineF nextLine = left ? QLineF (endOf (other), startOf (other)) :
                                     QLineF (startOf (other), endOf (other));
      const double angle = currentLine.angleTo (nextLine);
      const double turn = qMin (angle, 360.0 - angle);
      if (turn > m_maxTurnDegrees) {
        rejectedAtAngle = true;
        continue;
      }

      if (turn < bestAngle || (qAbs (turn - bestAngle) < 1e-9 && gap < bestGap)) {
        neighbor = other;
        bestAngle = turn;
        bestGap = gap;
      }
    }
  }

  if (neighbor == nullptr) {
    m_stoppedAtGap |= rejectedAtGap;
    m_stoppedAtAngle |= rejectedAtAngle;
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

QList<Segment*> SegmentChain::pathBetween (Segment *segmentStart,
                                           Segment *segmentEnd) const
{
  QList<Segment*> path;
  if (segmentStart == nullptr || segmentEnd == nullptr) {
    return path;
  }
  if (segmentStart == segmentEnd) {
    path << segmentStart;
    return path;
  }

  // Links point from the left piece to the right piece. The resulting graph has no cycles,
  // but Dijkstra also lets us compare every branch at a crossing before choosing one.
  QHash<Segment*, QList<Segment*> > adjacency;
  for (int index = 0; index < m_links.count (); index++) {
    const SegmentChainLink &link = m_links.at (index);
    adjacency [link.segmentA].append (link.segmentB);
  }

  QHash<Segment*, double> bestCost;
  QHash<Segment*, Segment*> previous;
  std::priority_queue<PathCandidate, std::vector<PathCandidate>, PathCandidateGreater> queue;
  bestCost.insert (segmentStart, 0.0);
  queue.push (PathCandidate {0.0, segmentStart});

  while (!queue.empty ()) {
    const PathCandidate candidate = queue.top ();
    queue.pop ();
    if (candidate.cost > bestCost.value (candidate.segment)) {
      continue;
    }
    if (candidate.segment == segmentEnd) {
      break;
    }

    const QList<Segment*> nextSegments = adjacency.value (candidate.segment);
    for (int index = 0; index < nextSegments.count (); index++) {
      Segment *next = nextSegments.at (index);
      const QLineF gapLine (endOf (candidate.segment), startOf (next));
      const double gap = gapLine.length ();
      if (gap > m_maxGapPixels) {
        continue;
      }

      const QLineF currentLine (startOf (candidate.segment), endOf (candidate.segment));
      const QLineF nextLine (startOf (next), endOf (next));
      const double angle = currentLine.angleTo (nextLine);
      const double turn = qMin (angle, 360.0 - angle);
      if (turn > m_maxTurnDegrees) {
        continue;
      }

      const double gapRatio = gap / qMax (1.0, m_maxGapPixels);
      const double turnRatio = turn / qMax (1.0, m_maxTurnDegrees);
      const double cost = candidate.cost + 0.1 +
                          2.0 * gapRatio * gapRatio +
                          4.0 * turnRatio * turnRatio;
      if (!bestCost.contains (next) || cost < bestCost.value (next)) {
        bestCost.insert (next, cost);
        previous.insert (next, candidate.segment);
        queue.push (PathCandidate {cost, next});
      }
    }
  }

  if (!previous.contains (segmentEnd)) {
    return path;
  }
  for (Segment *current = segmentEnd; current != nullptr; current = previous.value (current, nullptr)) {
    path.prepend (current);
  }
  return path;
}

bool SegmentChain::stoppedAtAngle () const
{
  return m_stoppedAtAngle;
}

bool SegmentChain::stoppedAtGap () const
{
  return m_stoppedAtGap;
}
