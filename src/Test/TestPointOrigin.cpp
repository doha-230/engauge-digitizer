/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "DocumentSerialize.h"
#include "Point.h"
#include "PointOrigin.h"
#include <QtTest/QtTest>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "Test/TestPointOrigin.h"

const double TEST_ORDINAL = 1.0;
const double TEST_X = 12.0;
const double TEST_Y = 34.0;

QTEST_MAIN (TestPointOrigin)

TestPointOrigin::TestPointOrigin(QObject *parent) :
  QObject(parent)
{
}

void TestPointOrigin::initTestCase ()
{
}

void TestPointOrigin::cleanupTestCase ()
{
}

void TestPointOrigin::testDefaultIsUnknown ()
{
  Point point (QString ("Curve1"),
               QPointF (TEST_X, TEST_Y),
               TEST_ORDINAL);

  QCOMPARE (signed (point.origin ()), signed (POINT_ORIGIN_UNKNOWN));
  QCOMPARE (pointOriginToString (point.origin ()), POINT_ORIGIN_VALUE_UNKNOWN);
}

void TestPointOrigin::testIsAutomated ()
{
  QVERIFY (!pointOriginIsAutomated (POINT_ORIGIN_UNKNOWN));
  QVERIFY (!pointOriginIsAutomated (POINT_ORIGIN_MANUAL));
  QVERIFY (pointOriginIsAutomated (POINT_ORIGIN_SEGMENT_FILL));
  QVERIFY (pointOriginIsAutomated (POINT_ORIGIN_POINT_MATCH));
  QVERIFY (!pointOriginIsAutomated (POINT_ORIGIN_PASTED));
}

void TestPointOrigin::testRoundTrip ()
{
  const PointOrigin origins[] = { POINT_ORIGIN_MANUAL,
                                  POINT_ORIGIN_SEGMENT_FILL,
                                  POINT_ORIGIN_POINT_MATCH,
                                  POINT_ORIGIN_PASTED };

  for (unsigned int index = 0; index < sizeof (origins) / sizeof (origins [0]); index++) {

    Point pointSaved (QString ("Curve1"),
                      QPointF (TEST_X, TEST_Y),
                      TEST_ORDINAL);
    pointSaved.setOrigin (origins [index]);

    // Serialize
    QString xml;
    {
      QXmlStreamWriter writer (&xml);
      writer.writeStartDocument ();
      pointSaved.saveXml (writer);
      writer.writeEndDocument ();
    }

    // Deserialize
    QXmlStreamReader reader (xml);
    Point pointLoaded;
    bool foundPoint = false;
    while (!reader.atEnd ()) {
      reader.readNext ();
      if (reader.isStartElement () && reader.name () == DOCUMENT_SERIALIZE_POINT) {
        pointLoaded = Point (reader);
        foundPoint = true;
        break;
      }
    }

    QVERIFY (foundPoint);
    QCOMPARE (signed (pointLoaded.origin ()), signed (origins [index]));
    QCOMPARE (pointOriginToString (pointLoaded.origin ()),
              pointOriginToString (origins [index]));
  }
}

void TestPointOrigin::testUnknownOriginIsNotWritten ()
{
  // A point whose origin is unknown must serialize exactly like the documents written
  // before this attribute existed, so old and new builds can read each other's files
  Point point (QString ("Curve1"),
               QPointF (TEST_X, TEST_Y),
               TEST_ORDINAL);

  QString xml;
  {
    QXmlStreamWriter writer (&xml);
    point.saveXml (writer);
  }

  QVERIFY (!xml.contains (DOCUMENT_SERIALIZE_POINT_ORIGIN));

  // An unknown name (or a name from a future build) reads back as unknown instead of failing
  QCOMPARE (signed (pointOriginFromString (QString ("SomethingElse"))),
            signed (POINT_ORIGIN_UNKNOWN));
  QCOMPARE (signed (pointOriginFromString (QString (""))),
            signed (POINT_ORIGIN_UNKNOWN));
}
