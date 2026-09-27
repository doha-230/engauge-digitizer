/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#ifndef TEST_QUALITY_REPORT_H
#define TEST_QUALITY_REPORT_H

#include <QObject>

/// Unit tests of the digitizing quality report checks
class TestQualityReport : public QObject
{
  Q_OBJECT
public:
  /// Single constructor.
  explicit TestQualityReport(QObject *parent = 0);

private slots:
  void cleanupTestCase ();
  void initTestCase ();

  void testCleanCurveHasNoIssues ();
  void testOutsideAxesIsReported ();
  void testOverlapBetweenCurvesIsReported ();
  void testRepeatedAndReversedXAreReported ();
  void testSteepChangeIsReported ();

private:

};

#endif // TEST_QUALITY_REPORT_H
