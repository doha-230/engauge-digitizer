/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#ifndef AUTOSAVE_RECOVERY_H
#define AUTOSAVE_RECOVERY_H

#include <QString>
#include <QStringList>

/// Autosave and crash recovery support (opt-in).
///
/// While a document is modified, a recovery copy is written next to the user's application data so
/// that an unexpected exit does not lose the digitizing work. The recovery file is a normal Engauge
/// document (.dig), which embeds the image, so recovering never depends on the original image file.
///
/// Everything here is local file access only: nothing is sent anywhere, which is what a closed
/// network deployment requires. The feature is off by default, and with it off this class is never
/// called.
class AutosaveRecovery
{
public:
  /// Single constructor
  AutosaveRecovery ();

  /// True when the autosave preference is enabled. Opt-in: false by default
  static bool enabled ();

  /// Autosave interval in minutes, at least 1. Defaults to 5 minutes
  static int intervalMinutes ();

  /// Directory holding the recovery files, or empty when no writable location is available
  static QString recoveryDirectory ();

  /// Recovery file that belongs to the specified document. A document that has never been saved
  /// uses the "untitled" key, so every unsaved document shares one recovery file.
  static QString recoveryFilePath (const QString &documentPath);

  /// Existing recovery files, newest first
  static QStringList recoveryFiles ();

  /// Remove the recovery file of the specified document, after a successful save or on exit
  static bool removeRecoveryFile (const QString &documentPath);

  /// Remove every recovery file. Used by the unit test and by a full cleanup
  static int removeAllRecoveryFiles ();

private:
  AutosaveRecovery (const AutosaveRecovery &other);

  /// Writable base directory for application data, or empty when none is writable
  static QString writableBaseDirectory ();
};

#endif // AUTOSAVE_RECOVERY_H
