/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "TemplateManager.h"
#include "CallbackCollectAxisPoints.h"
#include "CmdMediator.h"
#include "Document.h"
#include "functor.h"
#include "Logger.h"
#include "MainWindow.h"
#include "Point.h"
#include "Settings.h"
#include <QFile>
#include <QList>
#include <QSettings>

TemplateManager::TemplateManager ()
{
}

bool TemplateManager::applyTemplateToCurrentDocument (MainWindow &mainWindow,
                                                      const QString &templateFile)
{
  LOG4CPP_INFO_S ((*mainCat)) << "TemplateManager::applyTemplateToCurrentDocument"
                              << " template=" << templateFile.toLatin1().data();

  QFile file (templateFile);
  if (!file.exists ()) {
    LOG4CPP_ERROR_S ((*mainCat)) << "TemplateManager::applyTemplateToCurrentDocument template missing";
    return false;
  }

  // The template knowledge lives in a normal document, so it is loaded like any other document. The
  // separate CmdMediator keeps the template state apart from the current document state
  CmdMediator cmdTemplate (mainWindow,
                           templateFile);
  if (!cmdTemplate.reasonForUnsuccessfulRead ().isEmpty ()) {
    LOG4CPP_ERROR_S ((*mainCat)) << "TemplateManager::applyTemplateToCurrentDocument cannot read template"
                                 << cmdTemplate.reasonForUnsuccessfulRead().toLatin1().data();
    return false;
  }

  const Document &documentTemplate = cmdTemplate.document();
  Document &document = mainWindow.cmdMediator()->document();

  // Digitizing knowledge: models. Each setter goes through the main window update, which is what
  // keeps the scene and the dialogs in step
  mainWindow.updateSettingsCoords (documentTemplate.modelCoords ());
  mainWindow.updateSettingsColorFilter (documentTemplate.modelColorFilter ());
  mainWindow.updateSettingsGridRemoval (documentTemplate.modelGridRemoval ());
  mainWindow.updateSettingsSegments (documentTemplate.modelSegments ());
  mainWindow.updateSettingsPointMatch (documentTemplate.modelPointMatch ());
  mainWindow.updateSettingsExportFormat (documentTemplate.modelExport ());

  // Axis points: the template graph coordinates are copied onto the axis points of the current
  // document, in ordinal order, since the coordinates define the coordinate system
  QList<Point> axisPointsTemplate;
  {
    CallbackCollectAxisPoints ftor (axisPointsTemplate);
    Functor2wRet<const QString &, const Point &, CallbackSearchReturn> ftorWithCallback = functor_ret (ftor,
                                                                                                       &CallbackCollectAxisPoints::callback);
    documentTemplate.curveAxes ().iterateThroughCurvePoints (ftorWithCallback);
  }

  QList<Point> axisPointsCurrent;
  {
    CallbackCollectAxisPoints ftor (axisPointsCurrent);
    Functor2wRet<const QString &, const Point &, CallbackSearchReturn> ftorWithCallback = functor_ret (ftor,
                                                                                                       &CallbackCollectAxisPoints::callback);
    document.curveAxes ().iterateThroughCurvePoints (ftorWithCallback);
  }

  const int count = qMin (axisPointsTemplate.count (), axisPointsCurrent.count ());
  for (int index = 0; index < count; index++) {

    const Point &pointTemplate = axisPointsTemplate.at (index);
    Point &pointCurrent = axisPointsCurrent [index];

    // The template graph coordinates define the coordinate system. The template screen position is
    // copied as well, so the axis point stays where the user digitized it in the template image
    pointCurrent.setPosGraph (pointTemplate.posGraph ());
    pointCurrent.setPosScreen (pointTemplate.posScreen ());
  }

  mainWindow.updateAfterCommand ();

  return true;
}

QStringList TemplateManager::recentTemplates ()
{
  QSettings settings (SETTINGS_ENGAUGE, SETTINGS_DIGITIZER);
  settings.beginGroup (SETTINGS_GROUP_GENERAL);

  return settings.value (SETTINGS_GENERAL_RECENT_TEMPLATES,
                         QVariant (QStringList ())).toStringList ();
}

void TemplateManager::rememberTemplate (const QString &templateFile,
                                        int recentCount)
{
  QSettings settings (SETTINGS_ENGAUGE, SETTINGS_DIGITIZER);
  settings.beginGroup (SETTINGS_GROUP_GENERAL);

  QStringList recent = settings.value (SETTINGS_GENERAL_RECENT_TEMPLATES,
                                       QVariant (QStringList ())).toStringList ();

  recent.removeAll (templateFile);
  recent.prepend (templateFile);
  while (recent.count () > recentCount) {
    recent.removeLast ();
  }

  settings.setValue (SETTINGS_GENERAL_RECENT_TEMPLATES,
                     QVariant (recent));
  settings.endGroup ();
}
