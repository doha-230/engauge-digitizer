/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#ifndef TEST_AXIS_POINTS_VALIDATOR_H
#define TEST_AXIS_POINTS_VALIDATOR_H

#include <QObject>

/// Unit tests of the axis point review checks
class TestAxisPointsValidator : public QObject
{
  Q_OBJECT
public:
  /// Single constructor.
  explicit TestAxisPointsValidator(QObject *parent = 0);

private slots:
  void cleanupTestCase ();
  void initTestCase ();

  void testCleanAxisPointsProduceNoFindings ();
  void testDuplicateGraphCoordinatesAreReported ();
  void testXReversalIsReported ();
  void testYReversalIsReported ();
  void testIncompletePointsAreSkipped ();

private:

};

#endif // TEST_AXIS_POINTS_VALIDATOR_H
