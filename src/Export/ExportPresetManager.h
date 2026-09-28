/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#ifndef EXPORT_PRESET_MANAGER_H
#define EXPORT_PRESET_MANAGER_H

#include <QString>
#include <QStringList>

class DocumentModelExportFormat;

/// Named export format presets, stored in the settings as one entry per preset. The default state is
/// an empty list, which keeps the upstream behavior: no presets, no extra controls to notice.
///
/// Each preset stores the complete DocumentModelExportFormat in its own XML serialization, so a
/// preset survives format evolution as long as loadXml keeps accepting old files.
class ExportPresetManager
{
public:
  /// Remove the preset with this name. Returns true when it existed.
  static bool removePreset (const QString &presetName);

  /// Save the model under this name, replacing an existing preset of the same name
  static void savePreset (const QString &presetName,
                          const DocumentModelExportFormat &modelExport);

  /// Restore the preset with this name into the model. Returns true when it existed.
  static bool restorePreset (const QString &presetName,
                             DocumentModelExportFormat &modelExport);

  /// Preset names, sorted alphabetically
  static QStringList presetNames ();

private:
  ExportPresetManager ();
};

#endif // EXPORT_PRESET_MANAGER_H
