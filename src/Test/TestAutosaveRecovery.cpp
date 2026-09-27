/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "AutosaveRecovery.h"
#include "Settings.h"
#include <QtTest/QtTest>
#include <QDir>
#include <QFile>
#include <QSettings>
#include "Test/TestAutosaveRecovery.h"

QTEST_APPLESS_MAIN (TestAutosaveRecovery)

TestAutosaveRecovery::TestAutosaveRecovery(QObject *parent) :
  QObject(parent)
{
}

void TestAutosaveRecovery::initTestCase ()
{
}

void TestAutosaveRecovery::cleanupTestCase ()
{
  // Tests must not leave settings or recovery files behind
  {
    QSettings settings (SETTINGS_ENGAUGE, SETTINGS_DIGITIZER);
    settings.beginGroup (SETTINGS_GROUP_GENERAL);
    settings.remove (SETTINGS_GENERAL_AUTOSAVE_ENABLED);
    settings.remove (SETTINGS_GENERAL_AUTOSAVE_INTERVAL_MINUTES);
    settings.endGroup ();
  }

  AutosaveRecovery::removeRecoveryFile (QString ("engauge-unit-test.dig"));
}

void TestAutosaveRecovery::testEnabledFollowsSetting ()
{
  // Opt-in: with no setting stored, autosave is off
  {
    QSettings settings (SETTINGS_ENGAUGE, SETTINGS_DIGITIZER);
    settings.beginGroup (SETTINGS_GROUP_GENERAL);
    settings.remove (SETTINGS_GENERAL_AUTOSAVE_ENABLED);
    settings.endGroup ();
  }
  QVERIFY (!AutosaveRecovery::enabled ());

  {
    QSettings settings (SETTINGS_ENGAUGE, SETTINGS_DIGITIZER);
    settings.beginGroup (SETTINGS_GROUP_GENERAL);
    settings.setValue (SETTINGS_GENERAL_AUTOSAVE_ENABLED, true);
    settings.endGroup ();
  }
  QVERIFY (AutosaveRecovery::enabled ());
}

void TestAutosaveRecovery::testIntervalMinimum ()
{
  {
    QSettings settings (SETTINGS_ENGAUGE, SETTINGS_DIGITIZER);
    settings.beginGroup (SETTINGS_GROUP_GENERAL);
    settings.setValue (SETTINGS_GENERAL_AUTOSAVE_INTERVAL_MINUTES, 0);
    settings.endGroup ();
  }

  // An interval below one minute would autosave continuously, so it is clamped
  QCOMPARE (AutosaveRecovery::intervalMinutes (), 1);

  {
    QSettings settings (SETTINGS_ENGAUGE, SETTINGS_DIGITIZER);
    settings.beginGroup (SETTINGS_GROUP_GENERAL);
    settings.setValue (SETTINGS_GENERAL_AUTOSAVE_INTERVAL_MINUTES, 15);
    settings.endGroup ();
  }

  QCOMPARE (AutosaveRecovery::intervalMinutes (), 15);
}

void TestAutosaveRecovery::testRecoveryFilePathIsStableAndUnique ()
{
  const QString first = AutosaveRecovery::recoveryFilePath (QString ("C:/data/graph.dig"));
  const QString firstAgain = AutosaveRecovery::recoveryFilePath (QString ("C:/data/graph.dig"));
  const QString otherName = AutosaveRecovery::recoveryFilePath (QString ("C:/data/other.dig"));
  const QString otherDirectory = AutosaveRecovery::recoveryFilePath (QString ("D:/graphs/graph.dig"));
  const QString untitled = AutosaveRecovery::recoveryFilePath (QString ());

  // Stable
  QCOMPARE (first, firstAgain);

  // One recovery file per document, even when base names or directories are reused
  QVERIFY (first != otherName);
  QVERIFY (first != otherDirectory);
  QVERIFY (first != untitled);

  // The file name stays recognizable and keeps the dedicated suffix
  QVERIFY (first.contains ("graph.dig"));
  QVERIFY (first.endsWith (".dig.autosave"));
  QVERIFY (untitled.contains ("untitled"));
}

void TestAutosaveRecovery::testRecoveryFileLifeCycle ()
{
  const QString documentPath ("C:/data/engauge-unit-test.dig");
  const QString recoveryPath = AutosaveRecovery::recoveryFilePath (documentPath);
  const QString directory = AutosaveRecovery::recoveryDirectory ();

  if (directory.isEmpty ()) {
    QSKIP ("No writable location is available, so recovery cannot be tested");
  }

  QVERIFY (QDir ().mkpath (directory));

  {
    QFile file (recoveryPath);
    QVERIFY (file.open (QIODevice::WriteOnly));
    file.write ("recovery");
    file.close ();
  }

  // The recovery file is listed, and removing it makes it disappear
  QVERIFY (AutosaveRecovery::recoveryFiles ().contains (recoveryPath));
  QVERIFY (AutosaveRecovery::removeRecoveryFile (documentPath));
  QVERIFY (!QFile::exists (recoveryPath));
  QVERIFY (!AutosaveRecovery::recoveryFiles ().contains (recoveryPath));

  // Removing a file that is not there is not an error
  QVERIFY (!AutosaveRecovery::removeRecoveryFile (documentPath));
}
