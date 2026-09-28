/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#ifndef TEST_EXPORT_PRESET_H
#define TEST_EXPORT_PRESET_H

#include <QObject>

/// Unit tests of the export preset storage
class TestExportPreset : public QObject
{
  Q_OBJECT
public:
  /// Single constructor.
  explicit TestExportPreset(QObject *parent = 0);

private slots:
  void cleanup ();
  void cleanupTestCase ();
  void initTestCase ();

  void testEmptyStart ();
  void testSaveRestoreRoundTrip ();
  void testOverwriteSameName ();
  void testRemoveMissingIsFalse ();
  void testNamesAreSorted ();

private:

};

#endif // TEST_EXPORT_PRESET_H
