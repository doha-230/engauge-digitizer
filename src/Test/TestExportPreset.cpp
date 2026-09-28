/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "DocumentModelExportFormat.h"
#include "ExportPresetManager.h"
#include "Logger.h"
#include "Settings.h"
#include <QSettings>
#include <QTemporaryDir>
#include <QtTest/QtTest>
#include "Test/TestExportPreset.h"

QTEST_MAIN (TestExportPreset)

TestExportPreset::TestExportPreset(QObject *parent) :
  QObject(parent)
{
}

void TestExportPreset::initTestCase ()
{
  // The preset manager reads and writes the user settings, so the tests redirect them to a temporary
  // directory to stay out of the real profile
  QTemporaryDir *tempDir = new QTemporaryDir;
  QVERIFY (tempDir->isValid ());

  QSettings::setDefaultFormat (QSettings::IniFormat);
  QSettings::setPath (QSettings::IniFormat,
                      QSettings::UserScope,
                      tempDir->path ());

  initializeLogging ("engauge_test",
                     "engauge_test.log",
                     false); // NO_DEBUG
}

void TestExportPreset::cleanupTestCase ()
{
}

void TestExportPreset::cleanup ()
{
  // Remove every preset between tests, so each test starts from the empty state
  const QStringList names = ExportPresetManager::presetNames ();
  for (int index = 0; index < names.count (); index++) {
    ExportPresetManager::removePreset (names.at (index));
  }
}

void TestExportPreset::testEmptyStart ()
{
  // The default is an empty preset list, which is the upstream behavior
  QCOMPARE (ExportPresetManager::presetNames ().count (), 0);
}

void TestExportPreset::testSaveRestoreRoundTrip ()
{
  DocumentModelExportFormat modelExport;
  modelExport.setPointsSelectionFunctions (EXPORT_POINTS_SELECTION_FUNCTIONS_INTERPOLATE_ALL_CURVES);

  ExportPresetManager::savePreset ("My Preset",
                                   modelExport);

  DocumentModelExportFormat restored;
  QVERIFY (ExportPresetManager::restorePreset ("My Preset",
                                               restored));

  QCOMPARE (restored.pointsSelectionFunctions (),
            EXPORT_POINTS_SELECTION_FUNCTIONS_INTERPOLATE_ALL_CURVES);
}

void TestExportPreset::testOverwriteSameName ()
{
  DocumentModelExportFormat first;
  first.setPointsSelectionFunctions (EXPORT_POINTS_SELECTION_FUNCTIONS_INTERPOLATE_FIRST_CURVE);

  DocumentModelExportFormat second;
  second.setPointsSelectionFunctions (EXPORT_POINTS_SELECTION_FUNCTIONS_RAW);

  ExportPresetManager::savePreset ("Same Name",
                                   first);
  ExportPresetManager::savePreset ("Same Name",
                                   second);

  DocumentModelExportFormat restored;
  QVERIFY (ExportPresetManager::restorePreset ("Same Name",
                                               restored));

  QCOMPARE (restored.pointsSelectionFunctions (),
            EXPORT_POINTS_SELECTION_FUNCTIONS_RAW);
  QCOMPARE (ExportPresetManager::presetNames ().count (), 1);
}

void TestExportPreset::testRemoveMissingIsFalse ()
{
  QVERIFY (!ExportPresetManager::removePreset ("Does Not Exist"));
}

void TestExportPreset::testNamesAreSorted ()
{
  DocumentModelExportFormat modelExport;

  ExportPresetManager::savePreset ("zebra", modelExport);
  ExportPresetManager::savePreset ("alpha", modelExport);
  ExportPresetManager::savePreset ("middle", modelExport);

  const QStringList names = ExportPresetManager::presetNames ();

  QCOMPARE (names.at (0), QString ("alpha"));
  QCOMPARE (names.at (1), QString ("middle"));
  QCOMPARE (names.at (2), QString ("zebra"));
}
