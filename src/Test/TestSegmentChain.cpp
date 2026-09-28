/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "Logger.h"
#include "SegmentChain.h"
#include <QtTest/QtTest>
#include "Test/TestSegmentChain.h"

#include "Segment.h"
#include "SegmentFactory.h"
#include "SegmentLine.h"
#include <QApplication>
#include <QGraphicsScene>

namespace {

// Test fixture that owns a scene and a few straight segments. Each segment is one horizontal line
// from (x1, y) to (x2, y), which is exactly what the scanner produces for a flat curve piece.
class ChainFixture {
public:
  ChainFixture () : m_scene ()
  {
  }

  ~ChainFixture ()
  {
    clear ();
  }

  Segment *addSegment (double x1, double x2, double y)
  {
    return addSlopedSegment (x1, y, x2, y);
  }

  Segment *addSlopedSegment (double x1, double y1, double x2, double y2)
  {
    Segment *segment = new Segment (m_scene, (int) y1, false);
    SegmentLine *line = new SegmentLine (m_scene, m_modelSegments, segment);
    line->setLine (QLineF (x1, y1, x2, y2));
    line->hide (); // The scene here is only a container

    segment->appendLineForTest (line);

    m_segments << segment;

    return segment;
  }

  void clear ()
  {
    for (int index = 0; index < m_segments.count (); index++) {
      delete m_segments.at (index);
    }
    m_segments.clear ();
  }

  QGraphicsScene m_scene;
  DocumentModelSegments m_modelSegments;
  QList<Segment*> m_segments;
};

SegmentChainLink makeLink (Segment *a, Segment *b)
{
  SegmentChainLink link;
  link.segmentA = a;
  link.segmentB = b;
  link.xContact = a->lastPoint ().x ();

  return link;
}

QList<SegmentChainLink> linksFor (const QList<QPair<Segment*, Segment*> > &pairs)
{
  QList<SegmentChainLink> links;
  for (int index = 0; index < pairs.count (); index++) {
    links << makeLink (pairs.at (index).first, pairs.at (index).second);
  }

  return links;
}

}

QTEST_MAIN (TestSegmentChain)

TestSegmentChain::TestSegmentChain(QObject *parent) :
  QObject(parent)
{
}

void TestSegmentChain::initTestCase ()
{
  // The segment classes log through the LOG4CPP macros, which dereference the global mainCat, so
  // logging has to be initialized first
  initializeLogging ("engauge_test",
                     "engauge_test.log",
                     false); // NO_DEBUG
}

void TestSegmentChain::cleanupTestCase ()
{
}

void TestSegmentChain::testSingleSegmentChain ()
{
  ChainFixture fixture;
  Segment *middle = fixture.addSegment (10, 20, 5);

  SegmentChain chain (QList<SegmentChainLink> (), 3.0, 45.0);
  QList<Segment*> pieces = chain.chainFrom (middle);

  // One piece alone is a complete chain
  QCOMPARE (pieces.count (), 1);
  QCOMPARE (pieces.at (0), middle);
  QVERIFY (!chain.stoppedAtGap ());
  QVERIFY (!chain.stoppedAtAngle ());
}

void TestSegmentChain::testChainAcrossSmallGap ()
{
  ChainFixture fixture;
  Segment *left = fixture.addSegment (0, 10, 5);
  Segment *middle = fixture.addSegment (12, 22, 5); // 2 px gap after the first piece
  Segment *right = fixture.addSegment (24, 34, 5); // 2 px gap again

  QList<QPair<Segment*, Segment*> > pairs;
  pairs << qMakePair (left, middle) << qMakePair (middle, right);

  SegmentChain chain (linksFor (pairs), 3.0, 45.0);
  QList<Segment*> pieces = chain.chainFrom (middle);

  // Clicking the middle piece collects the whole chain in curve order
  QCOMPARE (pieces.count (), 3);
  QCOMPARE (pieces.at (0), left);
  QCOMPARE (pieces.at (1), middle);
  QCOMPARE (pieces.at (2), right);
  QVERIFY (!chain.stoppedAtGap ());
}

void TestSegmentChain::testGapAboveMaximumStopsChain ()
{
  ChainFixture fixture;
  Segment *left = fixture.addSegment (0, 10, 5);
  Segment *right = fixture.addSegment (20, 30, 5); // 10 px gap, above the 3 px maximum

  QList<QPair<Segment*, Segment*> > pairs;
  pairs << qMakePair (left, right);

  SegmentChain chain (linksFor (pairs), 3.0, 45.0);
  QList<Segment*> pieces = chain.chainFrom (left);

  // The far piece stays out of the chain
  QCOMPARE (pieces.count (), 1);
  QVERIFY (chain.stoppedAtGap ());
}

void TestSegmentChain::testSharpAngleStopsChain ()
{
  // A second piece that starts by going steeply up is what a grid line branching off looks like
  ChainFixture fixture;
  Segment *flat = fixture.addSegment (0, 10, 5);
  Segment *steep = new Segment (fixture.m_scene, 5, false);
  {
    SegmentLine *line1 = new SegmentLine (fixture.m_scene, fixture.m_modelSegments, steep);
    line1->setLine (QLineF (12, 5, 13, 40)); // Nearly vertical
    line1->hide ();
    steep->appendLineForTest (line1);
    fixture.m_segments << steep;
  }

  QList<QPair<Segment*, Segment*> > pairs;
  pairs << qMakePair (flat, steep);

  SegmentChain chain (linksFor (pairs), 3.0, 45.0);
  QList<Segment*> pieces = chain.chainFrom (flat);

  // The walk stops at the sharp angle instead of following the vertical piece
  QCOMPARE (pieces.count (), 1);
  QVERIFY (chain.stoppedAtAngle ());
}

void TestSegmentChain::testSmallClockwiseTurnContinues ()
{
  ChainFixture fixture;
  Segment *first = fixture.addSlopedSegment (10, 30, 20, 20);
  Segment *second = fixture.addSlopedSegment (21, 20, 31, 15);
  QList<QPair<Segment*, Segment*> > pairs;
  pairs << qMakePair (first, second);

  SegmentChain chain (linksFor (pairs), 3.0, 45.0);
  QList<Segment*> pieces = chain.chainFrom (first);

  QCOMPARE (pieces.count (), 2);
  QCOMPARE (pieces.at (1), second);
  QVERIFY (!chain.stoppedAtAngle ());
}

void TestSegmentChain::testCrossingChoosesStraighterPiece ()
{
  ChainFixture fixture;
  Segment *first = fixture.addSlopedSegment (10, 30, 20, 20);
  Segment *branch = fixture.addSlopedSegment (21, 20, 31, 40);
  Segment *continuation = fixture.addSlopedSegment (22, 19, 32, 11);
  QList<QPair<Segment*, Segment*> > pairs;
  pairs << qMakePair (first, branch) << qMakePair (first, continuation);

  SegmentChain chain (linksFor (pairs), 3.0, 45.0);
  QList<Segment*> pieces = chain.chainFrom (first);

  QCOMPARE (pieces.count (), 2);
  QCOMPARE (pieces.at (1), continuation);
  QVERIFY (!chain.stoppedAtAngle ());
}

void TestSegmentChain::testEndpointPathChoosesSmoothRoute ()
{
  ChainFixture fixture;
  Segment *start = fixture.addSegment (10, 20, 20);
  Segment *detour = fixture.addSlopedSegment (21, 20, 30, 22);
  Segment *straight = fixture.addSegment (21, 30, 20);
  Segment *end = fixture.addSegment (31, 40, 20);
  QList<QPair<Segment*, Segment*> > pairs;
  pairs << qMakePair (start, detour) << qMakePair (detour, end)
        << qMakePair (start, straight) << qMakePair (straight, end);

  SegmentChain chain (linksFor (pairs), 3.0, 45.0);
  QList<Segment*> pieces = chain.pathBetween (start, end);

  QCOMPARE (pieces.count (), 3);
  QCOMPARE (pieces.at (0), start);
  QCOMPARE (pieces.at (1), straight);
  QCOMPARE (pieces.at (2), end);
}

void TestSegmentChain::testEndpointPathRejectsDisconnectedPieces ()
{
  ChainFixture fixture;
  Segment *start = fixture.addSegment (10, 20, 20);
  Segment *end = fixture.addSegment (35, 45, 20);
  QList<QPair<Segment*, Segment*> > pairs;
  pairs << qMakePair (start, end);

  SegmentChain chain (linksFor (pairs), 3.0, 45.0);
  QVERIFY (chain.pathBetween (start, end).isEmpty ());
}

void TestSegmentChain::testSameColumnContactIsLinked ()
{
  ChainFixture fixture;
  Segment *start = fixture.addSegment (10, 20, 20);
  Segment *end = fixture.addSegment (20, 30, 20);
  SegmentFactory factory (fixture.m_scene, false);
  SegmentChain chain (factory.chainLinks (fixture.m_segments, 3.0), 3.0, 45.0);

  QList<Segment*> pieces = chain.pathBetween (start, end);
  QCOMPARE (pieces.count (), 2);
  QCOMPARE (pieces.at (0), start);
  QCOMPARE (pieces.at (1), end);
}
