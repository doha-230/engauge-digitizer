/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#ifndef TEST_POINT_ORIGIN_H
#define TEST_POINT_ORIGIN_H

#include <QObject>

/// Unit tests of the optional point provenance metadata (PointOrigin)
class TestPointOrigin : public QObject
{
  Q_OBJECT
public:
  /// Single constructor.
  explicit TestPointOrigin(QObject *parent = 0);

private slots:
  void cleanupTestCase ();
  void initTestCase ();

  void testDefaultIsUnknown ();
  void testIsAutomated ();
  void testRoundTrip ();
  void testUnknownOriginIsNotWritten ();

private:

};

#endif // TEST_POINT_ORIGIN_H
