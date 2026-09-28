/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#ifndef TEMPLATE_MANAGER_H
#define TEMPLATE_MANAGER_H

#include <QString>
#include <QStringList>

class MainWindow;

/// Template knowledge shared by the File > New From Template menu entry and the -batchtemplate
/// command line mode.
///
/// A template is a normal Engauge document that holds the digitizing knowledge: the axis point graph
/// coordinates that define the coordinate system, the color filter, the grid removal settings, the
/// segment settings, the point match settings and the export format. Applying a template copies that
/// knowledge onto the current document, whose image was just imported.
///
/// The recent template list is an application preference, so it survives restarts. Everything works on
/// local files only, which is what a closed network deployment requires.
class TemplateManager
{
public:
  /// Apply the template knowledge in the specified document file onto the current document of the
  /// main window. Returns true when the template was readable and applied.
  static bool applyTemplateToCurrentDocument (MainWindow &mainWindow,
                                              const QString &templateFile);

  /// Recent template paths, newest first. Empty when nothing was saved yet.
  static QStringList recentTemplates ();

  /// Put the template path at the front of the recent list, dropping duplicates, and keep at most
  /// the recentCount newest entries.
  static void rememberTemplate (const QString &templateFile,
                                int recentCount = 5);

private:
  TemplateManager ();
};

#endif // TEMPLATE_MANAGER_H
