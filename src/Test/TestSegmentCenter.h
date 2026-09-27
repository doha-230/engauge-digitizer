/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#ifndef TEST_SEGMENT_CENTER_H
#define TEST_SEGMENT_CENTER_H

#include <QObject>

/// Unit tests of the band centering strategies. The fixture is a synthetic image with a thick band,
/// and the expectations are the known centers of that band.
class TestSegmentCenter : public QObject
{
  Q_OBJECT
public:
  /// Single constructor.
  explicit TestSegmentCenter(QObject *parent = 0);

private slots:
  void cleanupTestCase ();
  void initTestCase ();

  void testTracedPathKeepsPoints ();
  void testPixelRunCenterFindsBandMiddle ();
  void testWeightedCenterPrefersDarkMiddle ();
  void testOffPixelKeepsPoint ();
  void testStrategyStringRoundTrip ();

private:

};

#endif // TEST_SEGMENT_CENTER_H
