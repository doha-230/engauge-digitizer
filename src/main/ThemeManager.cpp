/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "ThemeManager.h"
#include "Logger.h"
#include "Settings.h"
#include <QApplication>
#include <QPalette>
#include <QSettings>
#include <QStyleFactory>

namespace {

// Dark palette tuned for readability: surfaces slightly lighter than the window background, text
// near white, and the highlight in a calm blue. Assigned colors (curve colors, point colors) are
// never adjusted
QPalette darkPalette ()
{
  QPalette palette;
  palette.setColor (QPalette::Window, QColor (53, 53, 53));
  palette.setColor (QPalette::WindowText, QColor (230, 230, 230));
  palette.setColor (QPalette::Base, QColor (35, 35, 35));
  palette.setColor (QPalette::AlternateBase, QColor (53, 53, 53));
  palette.setColor (QPalette::ToolTipBase, QColor (53, 53, 53));
  palette.setColor (QPalette::ToolTipText, QColor (230, 230, 230));
  palette.setColor (QPalette::Text, QColor (230, 230, 230));
  palette.setColor (QPalette::Button, QColor (53, 53, 53));
  palette.setColor (QPalette::ButtonText, QColor (230, 230, 230));
  palette.setColor (QPalette::BrightText, QColor (255, 80, 80));
  palette.setColor (QPalette::Link, QColor (42, 130, 218));
  palette.setColor (QPalette::Highlight, QColor (42, 130, 218));
  palette.setColor (QPalette::HighlightedText, QColor (255, 255, 255));
  palette.setColor (QPalette::Disabled, QPalette::Text, QColor (128, 128, 128));
  palette.setColor (QPalette::Disabled, QPalette::ButtonText, QColor (128, 128, 128));

  return palette;
}

QPalette lightPalette ()
{
  QPalette palette;
  palette.setColor (QPalette::Window, QColor (240, 240, 240));
  palette.setColor (QPalette::WindowText, QColor (0, 0, 0));
  palette.setColor (QPalette::Base, QColor (255, 255, 255));
  palette.setColor (QPalette::AlternateBase, QColor (240, 240, 240));
  palette.setColor (QPalette::ToolTipBase, QColor (255, 255, 220));
  palette.setColor (QPalette::ToolTipText, QColor (0, 0, 0));
  palette.setColor (QPalette::Text, QColor (0, 0, 0));
  palette.setColor (QPalette::Button, QColor (240, 240, 240));
  palette.setColor (QPalette::ButtonText, QColor (0, 0, 0));
  palette.setColor (QPalette::BrightText, QColor (255, 0, 0));
  palette.setColor (QPalette::Link, QColor (42, 130, 218));
  palette.setColor (QPalette::Highlight, QColor (42, 130, 218));
  palette.setColor (QPalette::HighlightedText, QColor (255, 255, 255));
  palette.setColor (QPalette::Disabled, QPalette::Text, QColor (128, 128, 128));
  palette.setColor (QPalette::Disabled, QPalette::ButtonText, QColor (128, 128, 128));

  return palette;
}

}

void ThemeManager::apply (const QString &asName)
{
  if (asName == "Light") {
    apply (THEME_LIGHT);
  } else if (asName == "Dark") {
    apply (THEME_DARK);
  } else {
    // Unknown names and System keep the platform look, which is the upstream appearance
    apply (THEME_SYSTEM);
  }
}

void ThemeManager::apply (Theme theme)
{
  LOG4CPP_INFO_S ((*mainCat)) << "ThemeManager::apply"
                              << " theme=" << themeName (theme).toLatin1().data();

  switch (theme) {
    case THEME_LIGHT:
      QApplication::setStyle (QStyleFactory::create ("Fusion"));
      QApplication::setPalette (lightPalette ());
      break;

    case THEME_DARK:
      QApplication::setStyle (QStyleFactory::create ("Fusion"));
      QApplication::setPalette (darkPalette ());
      break;

    case THEME_SYSTEM:
    default:
      // Restore the platform defaults
      QApplication::setStyle (QStyleFactory::create ("windowsvista"));
      QApplication::setPalette (QApplication::style ()->standardPalette ());
      break;
  }
}

QString ThemeManager::themeName (Theme theme)
{
  switch (theme) {
    case THEME_LIGHT:
      return "Light";
    case THEME_DARK:
      return "Dark";
    case THEME_SYSTEM:
    default:
      return "System";
  }
}

ThemeManager::Theme ThemeManager::theme ()
{
  QSettings settings (SETTINGS_ENGAUGE, SETTINGS_DIGITIZER);
  settings.beginGroup (SETTINGS_GROUP_MAIN_WINDOW);

  const QString asName = settings.value (SETTINGS_MAIN_WINDOW_THEME,
                                         QVariant ("System")).toString ();

  if (asName == "Light") {
    return THEME_LIGHT;
  } else if (asName == "Dark") {
    return THEME_DARK;
  } else {
    return THEME_SYSTEM;
  }
}
