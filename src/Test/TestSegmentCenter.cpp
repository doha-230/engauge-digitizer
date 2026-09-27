/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "SegmentCenter.h"
#include "SegmentCenterStrategy.h"
#include <QtTest/QtTest>
#include <QImage>
#include "Test/TestSegmentCenter.h"

namespace {

const int BAND_WIDTH = 40;
const int BAND_TOP = 8;
const int BAND_BOTTOM = 17; // Inclusive, so the band is 10 pixels tall and its middle is 12.5

// Image with one horizontal black band on a white background. The "traced path" of the fill sits in
// the top half of the band, which is what the upstream fill produces for a thick curve.
QImage makeBandImage ()
{
  QImage image (BAND_WIDTH, 30, QImage::Format_RGB32);
  image.fill (QColor (255, 255, 255));

  for (int x = 0; x < BAND_WIDTH; x++) {
    for (int y = BAND_TOP; y <= BAND_BOTTOM; y++) {
      image.setPixel (x, y, QColor (0, 0, 0).rgb ());
    }
  }

  return image;
}

// The traced path inside the top half of the band
QList<QPoint> makeTracedPoints ()
{
  QList<QPoint> points;
  points << QPoint (5, BAND_TOP + 1);
  points << QPoint (20, BAND_TOP + 1);
  points << QPoint (35, BAND_TOP + 1);

  return points;
}

}

QTEST_MAIN (TestSegmentCenter)

TestSegmentCenter::TestSegmentCenter(QObject *parent) :
  QObject(parent)
{
}

void TestSegmentCenter::initTestCase ()
{
}

void TestSegmentCenter::cleanupTestCase ()
{
}

void TestSegmentCenter::testTracedPathKeepsPoints ()
{
  QImage image = makeBandImage ();

  // The default strategy is the upstream behavior: the points do not move
  QList<QPoint> result = SegmentCenter::centerPoints (makeTracedPoints (),
                                                      image,
                                                      TRACED_PATH_CENTER);

  QVERIFY (result == makeTracedPoints ());
}

void TestSegmentCenter::testPixelRunCenterFindsBandMiddle ()
{
  QImage image = makeBandImage ();

  QList<QPoint> result = SegmentCenter::centerPoints (makeTracedPoints (),
                                                      image,
                                                      PIXEL_RUN_CENTER);

  // Every point moves to the middle of the 10 pixel band. The round half up of 12.5 is 13
  QCOMPARE (result.count (), 3);
  for (int index = 0; index < result.count (); index++) {
    QCOMPARE (result.at (index).y (), 13);
  }
}

void TestSegmentCenter::testWeightedCenterPrefersDarkMiddle ()
{
  QImage image = makeBandImage ();

  // The band is uniformly black, so the weighted center is the same as the plain center
  QList<QPoint> result = SegmentCenter::centerPoints (makeTracedPoints (),
                                                      image,
                                                      WEIGHTED_PIXEL_CENTER);

  QCOMPARE (result.count (), 3);
  for (int index = 0; index < result.count (); index++) {
    QCOMPARE (result.at (index).y (), 13);
  }
}

void TestSegmentCenter::testOffPixelKeepsPoint ()
{
  QImage image = makeBandImage ();

  // A point outside the band cannot be centered, so it stays where it was
  QList<QPoint> points;
  points << QPoint (5, 2); // Above the band, in the white background

  QList<QPoint> result = SegmentCenter::centerPoints (points,
                                                      image,
                                                      PIXEL_RUN_CENTER);

  QCOMPARE (result.at (0), QPoint (5, 2));
}

void TestSegmentCenter::testStrategyStringRoundTrip ()
{
  // The settings store the strategy as a string, and unknown names fall back to the upstream
  // behavior
  QCOMPARE (segmentCenterStrategyToString (PIXEL_RUN_CENTER), QString ("PixelRunCenter"));
  QCOMPARE (segmentCenterStrategyFromString ("WeightedPixelCenter"), WEIGHTED_PIXEL_CENTER);
  QCOMPARE (segmentCenterStrategyFromString ("TracedPath"), TRACED_PATH_CENTER);
  QCOMPARE (segmentCenterStrategyFromString ("somethingUnknown"), TRACED_PATH_CENTER);
}
