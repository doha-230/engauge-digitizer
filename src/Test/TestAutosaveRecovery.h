/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#ifndef TEST_AUTOSAVE_RECOVERY_H
#define TEST_AUTOSAVE_RECOVERY_H

#include <QObject>

/// Unit tests of the opt-in autosave and crash recovery support
class TestAutosaveRecovery : public QObject
{
  Q_OBJECT
public:
  /// Single constructor.
  explicit TestAutosaveRecovery(QObject *parent = 0);

private slots:
  void cleanupTestCase ();
  void initTestCase ();

  void testEnabledFollowsSetting ();
  void testIntervalMinimum ();
  void testRecoveryFilePathIsStableAndUnique ();
  void testRecoveryFileLifeCycle ();

private:

};

#endif // TEST_AUTOSAVE_RECOVERY_H
