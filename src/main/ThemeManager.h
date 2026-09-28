/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#ifndef THEME_MANAGER_H
#define THEME_MANAGER_H

#include <QString>

/// Application theme, applied through the Qt stylesheet mechanism. System (the default) changes
/// nothing, which keeps the upstream appearance. Light and Dark force a Fusion based palette so the
/// result is the same on every system.
///
/// User defined curve colors are never touched: the theme only affects the widgets and the
/// application assigned colors, which is what keeps a color calibrated scan readable in any theme.
class ThemeManager
{
public:
  enum Theme {
    THEME_SYSTEM,
    THEME_LIGHT,
    THEME_DARK
  };

  /// Apply the named theme. Unknown names keep the system theme.
  static void apply (const QString &asName);

  /// Apply one of the enumerated themes.
  static void apply (Theme theme);

  /// Name of the theme in the settings, one of System, Light or Dark.
  static QString themeName (Theme theme);

  /// Current theme from the settings. Defaults to System, which is the upstream appearance.
  static Theme theme ();

private:
  ThemeManager ();
};

#endif // THEME_MANAGER_H
