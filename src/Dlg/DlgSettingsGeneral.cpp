/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "ButtonWhatsThis.h"
#include "CmdMediator.h"
#include "CmdSettingsGeneral.h"
#include "DlgSettingsGeneral.h"
#include "AutosaveRecovery.h"
#include <QCheckBox>
#include "EngaugeAssert.h"
#include "Logger.h"
#include "MainWindow.h"
#include <QComboBox>
#include <QGraphicsScene>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <qmath.h>
#include <QPushButton>
#include "ThemeManager.h"
#include <QSettings>
#include <QSpinBox>
#include <QWhatsThis>
#include "Settings.h"

DlgSettingsGeneral::DlgSettingsGeneral(MainWindow &mainWindow) :
  DlgSettingsAbstractBase (tr ("General"),
                           "DlgSettingsGeneral",
                           mainWindow),
  m_modelGeneralBefore (nullptr),
  m_modelGeneralAfter (nullptr)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsGeneral::DlgSettingsGeneral";

  QWidget *subPanel = createSubPanel ();
  finishPanel (subPanel);
}

DlgSettingsGeneral::~DlgSettingsGeneral()
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsGeneral::~DlgSettingsGeneral";
}

void DlgSettingsGeneral::createControls (QGridLayout *layout,
                                         int &row)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsGeneral::createControls";

  QLabel *labelCursorSize = new QLabel (QString ("%1:").arg (tr ("Effective cursor size (pixels)")));
  layout->addWidget (labelCursorSize, row, 1);

  m_spinCursorSize = new QSpinBox;
  m_spinCursorSize->setMinimum (1);
  m_spinCursorSize->setWhatsThis (tr ("Effective Cursor Size\n\n"
                                      "This is the effective width and height of the cursor when clicking on a pixel that is "
                                      "not part of the background.\n\n"
                                      "This parameter is used in the Color Picker and Point Match modes"));
  connect (m_spinCursorSize, SIGNAL (valueChanged (int)), this, SLOT (slotCursorSize (int)));
  layout->addWidget (m_spinCursorSize, row++, 2);

  QLabel *labelExtraPrecision = new QLabel (QString ("%1:").arg (tr ("Extra precision (digits)")));
  layout->addWidget (labelExtraPrecision, row, 1);

  m_spinExtraPrecision = new QSpinBox;
  m_spinExtraPrecision->setMinimum (0);
  m_spinExtraPrecision->setWhatsThis (tr ("Extra Digits of Precision\n\n"
                                          "This is the number of additional digits of precision appended after the significant "
                                          "digits determined by the digitization accuracy at that point. The digitization accuracy "
                                          "at any point equals the change in graph coordinates from moving one pixel in each direction. "
                                          "Appending extra digits does not improve the accuracy of the numbers. More information can "
                                          "be found in discussions of accuracy versus precision.\n\n"
                                          "This parameter is used on the coordinates in the Status Bar and during Export"));
  connect (m_spinExtraPrecision, SIGNAL (valueChanged (int)), this, SLOT (slotExtraPrecision (int)));
  layout->addWidget (m_spinExtraPrecision, row++, 2);

  // The two point origin options below are application preferences, not document data, so
  // they are read from and written to the settings directly. Both are off by default, which
  // keeps the appearance and the commands identical to previous builds.
  m_chkShowPointOrigin = new QCheckBox (tr ("Show which points were placed automatically"));
  m_chkShowPointOrigin->setWhatsThis (tr ("Show Which Points Were Placed Automatically\n\n"
                                          "Draws points created by the Segment Fill and Point Match tools with a dashed "
                                          "outline, so they can be told apart from points placed by hand.\n\n"
                                          "This is useful after changing the color filter or the grid removal settings: "
                                          "the dashed points are the ones that were derived from those settings."));
  connect (m_chkShowPointOrigin, SIGNAL (toggled (bool)), this, SLOT (slotShowPointOrigin (bool)));
  layout->addWidget (m_chkShowPointOrigin, row++, 1, 1, 2);

  m_chkAxisValidation = new QCheckBox (tr ("Review the axis points when they look questionable"));
  m_chkAxisValidation->setWhatsThis (tr ("Review the Axis Points\n\n"
                                         "Shows a confirmation prompt before a coordinate system is used, when the axis points "
                                         "look questionable: two points with the same graph coordinates, or x or y values that "
                                         "go backwards, which suggests the points were digitized in the wrong order.\n\n"
                                         "The prompt is a reminder rather than an error, and it appears only when the axis points "
                                         "changed since the last review."));
  connect (m_chkAxisValidation, SIGNAL (toggled (bool)), this, SLOT (slotAxisValidation (bool)));
  layout->addWidget (m_chkAxisValidation, row++, 1, 1, 2);

  m_chkCompactZoom = new QCheckBox (tr ("Compact zoom controls"));
  m_chkCompactZoom->setWhatsThis (tr ("Compact Zoom Controls\n\n"
                                      "Replaces the twenty entry zoom list in the View menu with a compact zoom "
                                      "combo plus fill, zoom in and zoom out. Mouse wheel zooming, panning and the "
                                      "keyboard shortcuts work the same in both layouts."));
  connect (m_chkCompactZoom, SIGNAL (toggled (bool)), this, SLOT (slotCompactZoom (bool)));
  layout->addWidget (m_chkCompactZoom, row++, 1, 1, 2);

  QLabel *labelTheme = new QLabel (QString ("%1:").arg (tr ("Theme")));
  layout->addWidget (labelTheme, row, 0);

  m_cmbTheme = new QComboBox;
  m_cmbTheme->addItem (tr ("System"));
  m_cmbTheme->addItem (tr ("Light"));
  m_cmbTheme->addItem (tr ("Dark"));
  m_cmbTheme->setWhatsThis (tr ("Theme\n\n"
                                "Application colors. System (the default) uses the platform colors, which is the "
                                "appearance of the original application. Light and Dark force a consistent palette "
                                "independent of the platform.\n\n"
                                "Curve colors that the user assigned are never changed by the theme, which keeps a "
                                "color calibrated scan readable in every theme."));
  connect (m_cmbTheme, SIGNAL (currentTextChanged (const QString &)), this, SLOT (slotTheme (const QString &)));
  layout->addWidget (m_cmbTheme, row, 1);
  row++;

  m_chkAutosave = new QCheckBox (tr ("Auto-save a recovery file while the document is modified"));
  m_chkAutosave->setWhatsThis (tr ("Auto-save Recovery File\n\n"
                                   "Writes a recovery copy of the document at the interval below while there are "
                                   "unsaved changes, so an unexpected exit does not lose the digitizing work. "
                                   "The copy is a normal Engauge document that contains the image, and it is "
                                   "removed as soon as the document is saved or closed normally.\n\n"
                                   "Nothing leaves the machine: recovery files are written to the application data "
                                   "directory, or next to the application for a portable installation."));
  connect (m_chkAutosave, SIGNAL (toggled (bool)), this, SLOT (slotAutosave (bool)));
  layout->addWidget (m_chkAutosave, row++, 1, 1, 2);

  QLabel *labelAutosaveInterval = new QLabel (QString ("%1:").arg (tr ("Auto-save interval (minutes)")));
  layout->addWidget (labelAutosaveInterval, row, 1);

  m_spinAutosaveInterval = new QSpinBox;
  m_spinAutosaveInterval->setMinimum (1);
  m_spinAutosaveInterval->setMaximum (60);
  m_spinAutosaveInterval->setWhatsThis (tr ("Auto-save Interval\n\n"
                                            "Minutes between recovery file updates while the document is modified."));
  connect (m_spinAutosaveInterval, SIGNAL (valueChanged (int)), this, SLOT (slotAutosaveInterval (int)));
  layout->addWidget (m_spinAutosaveInterval, row++, 2);

  m_chkEnableRedigitize = new QCheckBox (tr ("Enable commands that work on automatically placed points"));
  m_chkEnableRedigitize->setWhatsThis (tr ("Enable Commands For Automatically Placed Points\n\n"
                                           "Adds Delete Automated Points to the Edit menu, which removes every point that "
                                           "was created by the Segment Fill or Point Match tools while keeping the points "
                                           "placed by hand. Digitize a curve again afterwards.\n\n"
                                           "This option is off by default so the menu stays unchanged."));
  connect (m_chkEnableRedigitize, SIGNAL (toggled (bool)), this, SLOT (slotEnableRedigitize (bool)));
  layout->addWidget (m_chkEnableRedigitize, row++, 1, 1, 2);
}

void DlgSettingsGeneral::createOptionalSaveDefault (QHBoxLayout *layout)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsGeneral::createOptionalSaveDefault";

  m_btnSaveDefault = new QPushButton (tr ("Save As Default"));
  m_btnSaveDefault->setWhatsThis (tr ("Save the settings for use as future defaults, according to the curve name selection."));
  connect (m_btnSaveDefault, SIGNAL (released ()), this, SLOT (slotSaveDefault ()));
  layout->addWidget (m_btnSaveDefault, 0, Qt::AlignLeft);
}

QWidget *DlgSettingsGeneral::createSubPanel ()
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsGeneral::createSubPanel";

  QWidget *subPanel = new QWidget ();
  QGridLayout *layout = new QGridLayout (subPanel);
  subPanel->setLayout (layout);

  layout->setColumnStretch(0, 1); // Empty first column
  layout->setColumnStretch(1, 0); // Labels
  layout->setColumnStretch(2, 0); // Values
  layout->setColumnStretch(3, 1); // Empty first column

  int row = 0;

  createWhatsThis (layout,
                   m_btnWhatsThis,
                   row++,
                   3);

  createControls (layout, row);

  return subPanel;
}

void DlgSettingsGeneral::handleOk ()
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsGeneral::handleOk";

  CmdSettingsGeneral *cmd = new CmdSettingsGeneral (mainWindow (),
                                                    cmdMediator ().document(),
                                                    *m_modelGeneralBefore,
                                                    *m_modelGeneralAfter);
  cmdMediator ().push (cmd);

  hide ();
}

void DlgSettingsGeneral::load (CmdMediator &cmdMediator)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsGeneral::load";

  setCmdMediator (cmdMediator);

  // Flush old data
  delete m_modelGeneralBefore;
  delete m_modelGeneralAfter;

  // Save new data
  m_modelGeneralBefore = new DocumentModelGeneral (cmdMediator.document());
  m_modelGeneralAfter = new DocumentModelGeneral (cmdMediator.document());

  // Populate controls
  m_spinCursorSize->setValue (m_modelGeneralAfter->cursorSize());
  m_spinExtraPrecision->setValue (m_modelGeneralAfter->extraPrecision());

  {
    // Application preferences, so they come from the settings instead of the document
    QSettings settings (SETTINGS_ENGAUGE, SETTINGS_DIGITIZER);
    settings.beginGroup (SETTINGS_GROUP_GENERAL);

    m_chkAxisValidation->setChecked (settings.value (SETTINGS_GENERAL_AXIS_VALIDATION,
                                                     QVariant (false)).toBool ());
    m_chkCompactZoom->setChecked (settings.value (SETTINGS_MAIN_WINDOW_COMPACT_ZOOM,
                                                  QVariant (false)).toBool ());
    m_cmbTheme->setCurrentText (settings.value (SETTINGS_MAIN_WINDOW_THEME,
                                                QVariant ("System")).toString ());
    m_chkAutosave->setChecked (settings.value (SETTINGS_GENERAL_AUTOSAVE_ENABLED,
                                               QVariant (false)).toBool ());
    m_spinAutosaveInterval->setValue (AutosaveRecovery::intervalMinutes ());
    m_spinAutosaveInterval->setEnabled (m_chkAutosave->isChecked ());

    m_chkShowPointOrigin->setChecked (settings.value (SETTINGS_GENERAL_SHOW_POINT_ORIGIN,
                                                      QVariant (false)).toBool ());
    m_chkEnableRedigitize->setChecked (settings.value (SETTINGS_GENERAL_ENABLE_REDIGITIZE,
                                                       QVariant (false)).toBool ());
    settings.endGroup ();
  }

  updateControls ();
  enableOk (false); // Disable Ok button since there not yet any changes
}

void DlgSettingsGeneral::setSmallDialogs(bool /* smallDialogs */)
{
}

void DlgSettingsGeneral::slotCompactZoom (bool compact)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsGeneral::slotCompactZoom";

  QSettings settings (SETTINGS_ENGAUGE, SETTINGS_DIGITIZER);
  settings.beginGroup (SETTINGS_GROUP_MAIN_WINDOW);
  settings.setValue (SETTINGS_MAIN_WINDOW_COMPACT_ZOOM,
                     compact);
  settings.endGroup ();
}

void DlgSettingsGeneral::slotTheme (const QString &theme)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsGeneral::slotTheme";

  QSettings settings (SETTINGS_ENGAUGE, SETTINGS_DIGITIZER);
  settings.beginGroup (SETTINGS_GROUP_MAIN_WINDOW);
  settings.setValue (SETTINGS_MAIN_WINDOW_THEME,
                     theme);
  settings.endGroup ();

  // Apply immediately, so the user sees the result without restarting
  ThemeManager::apply (theme);
}

void DlgSettingsGeneral::slotAxisValidation (bool review)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsGeneral::slotAxisValidation";

  QSettings settings (SETTINGS_ENGAUGE, SETTINGS_DIGITIZER);
  settings.beginGroup (SETTINGS_GROUP_GENERAL);
  settings.setValue (SETTINGS_GENERAL_AXIS_VALIDATION,
                     review);
  settings.endGroup ();
}

void DlgSettingsGeneral::slotAutosave (bool autosave)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsGeneral::slotAutosave";

  QSettings settings (SETTINGS_ENGAUGE, SETTINGS_DIGITIZER);
  settings.beginGroup (SETTINGS_GROUP_GENERAL);
  settings.setValue (SETTINGS_GENERAL_AUTOSAVE_ENABLED,
                     autosave);
  settings.endGroup ();

  m_spinAutosaveInterval->setEnabled (autosave);
}

void DlgSettingsGeneral::slotAutosaveInterval (int minutes)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsGeneral::slotAutosaveInterval";

  QSettings settings (SETTINGS_ENGAUGE, SETTINGS_DIGITIZER);
  settings.beginGroup (SETTINGS_GROUP_GENERAL);
  settings.setValue (SETTINGS_GENERAL_AUTOSAVE_INTERVAL_MINUTES,
                     minutes);
  settings.endGroup ();
}

void DlgSettingsGeneral::slotShowPointOrigin (bool show)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsGeneral::slotShowPointOrigin";

  QSettings settings (SETTINGS_ENGAUGE, SETTINGS_DIGITIZER);
  settings.beginGroup (SETTINGS_GROUP_GENERAL);
  settings.setValue (SETTINGS_GENERAL_SHOW_POINT_ORIGIN,
                     show);
  settings.endGroup ();
}

void DlgSettingsGeneral::slotEnableRedigitize (bool enable)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsGeneral::slotEnableRedigitize";

  QSettings settings (SETTINGS_ENGAUGE, SETTINGS_DIGITIZER);
  settings.beginGroup (SETTINGS_GROUP_GENERAL);
  settings.setValue (SETTINGS_GENERAL_ENABLE_REDIGITIZE,
                     enable);
  settings.endGroup ();
}

void DlgSettingsGeneral::slotCursorSize (int cursorSize)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsGeneral::slotCursorSize";

  m_modelGeneralAfter->setCursorSize (cursorSize);
  updateControls();
}

void DlgSettingsGeneral::slotExtraPrecision (int extraPrecision)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsGeneral::slotExtraPrecision";

  m_modelGeneralAfter->setExtraPrecision (extraPrecision);
  updateControls();
}

void DlgSettingsGeneral::slotSaveDefault()
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsGeneral::slotSaveDefault";

  QSettings settings (SETTINGS_ENGAUGE, SETTINGS_DIGITIZER);
  settings.beginGroup (SETTINGS_GROUP_GENERAL);

  settings.setValue (SETTINGS_GENERAL_CURSOR_SIZE,
                     m_modelGeneralAfter->cursorSize());
  settings.setValue (SETTINGS_GENERAL_EXTRA_PRECISION,
                     m_modelGeneralAfter->extraPrecision());
  settings.endGroup ();
}

void DlgSettingsGeneral::slotWhatsThis ()
{
  QWhatsThis::enterWhatsThisMode();
}

void DlgSettingsGeneral::updateControls ()
{
  enableOk (true);
}
