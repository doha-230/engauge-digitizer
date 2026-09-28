/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "ExportPresetManager.h"
#include "DocumentModelExportFormat.h"
#include "Logger.h"
#include "Settings.h"
#include <QBuffer>
#include <QSettings>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace {

// Every preset is one settings entry under this group: the name is the key, the value is the XML of
// the complete export format model
const QString GROUP ("ExportPresetsEngauge");

QString modelToXml (const DocumentModelExportFormat &modelExport)
{
  QByteArray bytes;
  QBuffer buffer (&bytes);
  buffer.open (QIODevice::WriteOnly);

  QXmlStreamWriter writer (&buffer);
  writer.setAutoFormatting (false);
  modelExport.saveXml (writer);

  return QString::fromUtf8 (bytes);
}

DocumentModelExportFormat modelFromXml (const QString &asXml)
{
  DocumentModelExportFormat modelExport;

  // loadXml reads the attributes of the CURRENT element, so the reader has to be advanced onto the
  // exported <export> element first
  QXmlStreamReader reader (asXml);
  while (!reader.atEnd () && !reader.hasError ()) {
    if (reader.readNext () == QXmlStreamReader::StartElement) {
      modelExport.loadXml (reader);
      break;
    }
  }

  return modelExport;
}

}

void ExportPresetManager::savePreset (const QString &presetName,
                                      const DocumentModelExportFormat &modelExport)
{
  LOG4CPP_INFO_S ((*mainCat)) << "ExportPresetManager::savePreset"
                              << " name=" << presetName.toLatin1().data();

  QSettings settings (SETTINGS_ENGAUGE, SETTINGS_DIGITIZER);
  settings.beginGroup (GROUP);
  settings.setValue (presetName,
                     QVariant (modelToXml (modelExport)));
  settings.endGroup ();
}

bool ExportPresetManager::restorePreset (const QString &presetName,
                                         DocumentModelExportFormat &modelExport)
{
  QSettings settings (SETTINGS_ENGAUGE, SETTINGS_DIGITIZER);
  settings.beginGroup (GROUP);

  const QVariant value = settings.value (presetName);
  settings.endGroup ();

  if (!value.isValid ()) {
    return false;
  }

  modelExport = modelFromXml (value.toString ());

  return true;
}

bool ExportPresetManager::removePreset (const QString &presetName)
{
  LOG4CPP_INFO_S ((*mainCat)) << "ExportPresetManager::removePreset"
                              << " name=" << presetName.toLatin1().data();

  QSettings settings (SETTINGS_ENGAUGE, SETTINGS_DIGITIZER);
  settings.beginGroup (GROUP);

  const bool existed = settings.contains (presetName);
  if (existed) {
    settings.remove (presetName);
  }
  settings.endGroup ();

  return existed;
}

QStringList ExportPresetManager::presetNames ()
{
  QSettings settings (SETTINGS_ENGAUGE, SETTINGS_DIGITIZER);
  settings.beginGroup (GROUP);

  QStringList names = settings.childKeys ();
  settings.endGroup ();

  names.sort ();

  return names;
}
