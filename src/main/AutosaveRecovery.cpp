/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "AutosaveRecovery.h"
#include "Crc32.h"
#include "Logger.h"
#include "Settings.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>
#include <QVariant>

namespace {

const QString RECOVERY_DIRECTORY ("recovery");
const QString RECOVERY_FILE_SUFFIX (".dig.autosave");
const QString UNTITLED_DOCUMENT ("untitled");
const QString WRITE_TEST_FILE ("engauge_write_test.tmp");
const int DEFAULT_AUTOSAVE_INTERVAL_MINUTES = 5;

}

AutosaveRecovery::AutosaveRecovery ()
{
}

bool AutosaveRecovery::enabled ()
{
  QSettings settings (SETTINGS_ENGAUGE, SETTINGS_DIGITIZER);
  settings.beginGroup (SETTINGS_GROUP_GENERAL);
  return settings.value (SETTINGS_GENERAL_AUTOSAVE_ENABLED,
                         QVariant (false)).toBool ();
}

int AutosaveRecovery::intervalMinutes ()
{
  QSettings settings (SETTINGS_ENGAUGE, SETTINGS_DIGITIZER);
  settings.beginGroup (SETTINGS_GROUP_GENERAL);
  const int interval = settings.value (SETTINGS_GENERAL_AUTOSAVE_INTERVAL_MINUTES,
                                       QVariant (DEFAULT_AUTOSAVE_INTERVAL_MINUTES)).toInt ();

  return interval < 1 ? 1 : interval;
}

QString AutosaveRecovery::writableBaseDirectory ()
{
  // Prefer the per-user application data location. Fall back to the directory of the
  // executable so that a portable deployment on a machine without a writable profile still
  // gets crash recovery instead of silently doing nothing.
  QStringList candidates;
  const QString appLocalData = QStandardPaths::writableLocation (QStandardPaths::AppLocalDataLocation);
  if (!appLocalData.isEmpty ()) {
    candidates << appLocalData;
  }
  candidates << QCoreApplication::applicationDirPath ();

  for (int index = 0; index < candidates.count (); index++) {
    const QString candidate = candidates.at (index);
    QDir dir (candidate);
    if (!dir.exists () && !dir.mkpath (".")) {
      continue;
    }

    // Test that the directory is really writable, since permission checks are unreliable
    QFile probe (dir.filePath (WRITE_TEST_FILE));
    if (probe.open (QIODevice::WriteOnly)) {
      probe.close ();
      probe.remove ();
      return candidate;
    }
  }

  LOG4CPP_WARN_S ((*mainCat)) << "AutosaveRecovery::writableBaseDirectory no writable directory";
  return QString ();
}

QString AutosaveRecovery::recoveryDirectory ()
{
  const QString base = writableBaseDirectory ();
  if (base.isEmpty ()) {
    return QString ();
  }

  return QString ("%1%2%3").arg (base).arg (QDir::separator ()).arg (RECOVERY_DIRECTORY);
}

QString AutosaveRecovery::recoveryFilePath (const QString &documentPath)
{
  // The document name keeps the file recognizable, and the checksum keeps documents with the
  // same base name but different locations apart
  const QString key = documentPath.isEmpty () ?
                      UNTITLED_DOCUMENT :
                      QFileInfo (documentPath).fileName ();

  Crc32 crc32;
  const QByteArray utf8 = documentPath.toUtf8 ();
  const unsigned crc = crc32.memcrc (reinterpret_cast<const unsigned char *> (utf8.constData ()),
                                     unsigned (utf8.size ()));

  const QString fileName = QString ("%1_%2%3")
                                   .arg (key)
                                   .arg (crc, 8, 16, QChar ('0'))
                                   .arg (RECOVERY_FILE_SUFFIX);

  const QString directory = recoveryDirectory ();
  if (directory.isEmpty ()) {
    return fileName;
  }

  return QDir (directory).filePath (fileName);
}

QStringList AutosaveRecovery::recoveryFiles ()
{
  QStringList files;

  const QString directory = recoveryDirectory ();
  if (directory.isEmpty ()) {
    return files;
  }

  QDir dir (directory);
  if (!dir.exists ()) {
    return files;
  }

  const QFileInfoList entries = dir.entryInfoList (QStringList ("*" + RECOVERY_FILE_SUFFIX),
                                                   QDir::Files,
                                                   QDir::Time); // Newest first
  for (int index = 0; index < entries.count (); index++) {
    files << entries.at (index).absoluteFilePath ();
  }

  return files;
}

bool AutosaveRecovery::removeRecoveryFile (const QString &documentPath)
{
  const QString fileName = recoveryFilePath (documentPath);

  return QFile::exists (fileName) && QFile::remove (fileName);
}

int AutosaveRecovery::removeAllRecoveryFiles ()
{
  int removed = 0;

  const QStringList files = recoveryFiles ();
  for (int index = 0; index < files.count (); index++) {
    if (QFile::remove (files.at (index))) {
      removed++;
    }
  }

  return removed;
}
