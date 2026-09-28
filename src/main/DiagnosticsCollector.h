/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#ifndef DIAGNOSTICS_COLLECTOR_H
#define DIAGNOSTICS_COLLECTOR_H

#include <QString>

class MainWindow;

/// Collects the diagnostic facts that an offline support conversation needs, as plain text:
/// the version, the Qt runtime, the build date, the operating system and screen scale, the current
/// document summary, and the tail of the application log.
///
/// The result goes to the clipboard and optionally to a file the user chooses. Nothing is sent
/// anywhere - this is the closed network version, so saving and copying are the only exits.
class DiagnosticsCollector
{
public:
  /// Build the diagnostics text for the current state of the main window.
  static QString collect (MainWindow &mainWindow);

private:
  DiagnosticsCollector ();
};

#endif // DIAGNOSTICS_COLLECTOR_H
