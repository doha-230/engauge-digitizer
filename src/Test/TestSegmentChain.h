/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#ifndef TEST_SEGMENT_CHAIN_H
#define TEST_SEGMENT_CHAIN_H

#include <QObject>

/// Unit tests of the segment chain walking used by the Auto Curve Detection mode. The chain graph
/// itself is plain data, so the tests build links directly without an image or a document.
class TestSegmentChain : public QObject
{
  Q_OBJECT
public:
  /// Single constructor.
  explicit TestSegmentChain(QObject *parent = 0);

private slots:
  void cleanupTestCase ();
  void initTestCase ();

  void testChainAcrossSmallGap ();
  void testGapAboveMaximumStopsChain ();
  void testSharpAngleStopsChain ();
  void testSingleSegmentChain ();

private:

};

#endif // TEST_SEGMENT_CHAIN_H
