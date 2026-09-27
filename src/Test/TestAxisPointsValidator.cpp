/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "AxisPointsValidator.h"
#include "Logger.h"
#include "Point.h"
#include <QtTest/QtTest>
#include "Test/TestAxisPointsValidator.h"

namespace {

Point makeAxisPoint (const QString &identifier,
                     double xGraph,
                     double yGraph,
                     bool isXOnly = false)
{
  return Point ("AxisCurve",
                identifier,
                QPointF (xGraph, yGraph), // Screen coordinates are unused by the checks
                QPointF (xGraph, yGraph),
                0.0,
                isXOnly);
}

int findingCount (const AxisPointsValidator &validator,
                  const QString &keyword)
{
  int count = 0;
  const QStringList &findings = validator.findings ();
  for (int index = 0; index < findings.count (); index++) {
    if (findings.at (index).contains (keyword)) {
      count++;
    }
  }

  return count;
}

}

QTEST_MAIN (TestAxisPointsValidator)

TestAxisPointsValidator::TestAxisPointsValidator(QObject *parent) :
  QObject(parent)
{
}

void TestAxisPointsValidator::initTestCase ()
{
  // The Point constructors log through the LOG4CPP macros, which dereference the global mainCat, so
  // logging has to be initialized before any Point is created
  initializeLogging ("engauge_test",
                     "engauge_test.log",
                     false); // NO_DEBUG
}

void TestAxisPointsValidator::cleanupTestCase ()
{
}

void TestAxisPointsValidator::testCleanAxisPointsProduceNoFindings ()
{
  QList<Point> points;
  points << makeAxisPoint ("X1", 100, 0, true)
         << makeAxisPoint ("X2", 200, 0, true)
         << makeAxisPoint ("Y1", 0, 100)
         << makeAxisPoint ("Y2", 0, 200);

  AxisPointsValidator validator (points);

  // Rising x and rising y are correct orders, and nothing is duplicated
  QCOMPARE (validator.findings ().count (), 0);
}

void TestAxisPointsValidator::testDuplicateGraphCoordinatesAreReported ()
{
  QList<Point> points;
  points << makeAxisPoint ("A1", 100, 200)
         << makeAxisPoint ("A2", 100, 200); // Same coordinates as A1

  AxisPointsValidator validator (points);

  QCOMPARE (findingCount (validator, "same graph coordinates"), 1);
}

void TestAxisPointsValidator::testXReversalIsReported ()
{
  QList<Point> points;
  points << makeAxisPoint ("X1", 300, 0, true)
         << makeAxisPoint ("X2", 100, 0, true); // x goes backwards

  AxisPointsValidator validator (points);

  QCOMPARE (findingCount (validator, "X axis point"), 1);
}

void TestAxisPointsValidator::testYReversalIsReported ()
{
  QList<Point> points;
  points << makeAxisPoint ("Y1", 0, 300)
         << makeAxisPoint ("Y2", 0, 100); // y goes downwards

  AxisPointsValidator validator (points);

  QCOMPARE (findingCount (validator, "Y axis point"), 1);
}

void TestAxisPointsValidator::testIncompletePointsAreSkipped ()
{
  // X-only points have no meaningful y, so they cannot be duplicate checked or y ordered, and they
  // must not crash or produce findings on their own
  QList<Point> points;
  points << makeAxisPoint ("X1", 100, 0, true)
         << makeAxisPoint ("X2", 200, 0, true);

  AxisPointsValidator validator (points);

  QCOMPARE (validator.findings ().count (), 0);
}
