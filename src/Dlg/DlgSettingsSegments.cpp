/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "ButtonWhatsThis.h"
#include "CmdMediator.h"
#include "CmdSettingsSegments.h"
#include "DlgSettingsSegments.h"
#include "EngaugeAssert.h"
#include "GeometryWindow.h"
#include "InactiveOpacity.h"
#include "Logger.h"
#include "MainWindow.h"
#include "PointStyle.h"
#include "SegmentCenterStrategy.h"
#include "Settings.h"
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QSettings>
#include <QGraphicsScene>
#include <QLabel>
#include <qmath.h>
#include <QSpinBox>
#include <QWhatsThis>
#include "Segment.h"
#include "SegmentFactory.h"
#include "ViewPreview.h"

const int MINIMUM_HEIGHT = 540;
const int MIN_LENGTH_MIN = 1;
const int MIN_LENGTH_MAX = 10000;
const int POINT_SEPARATION_MIN = 5;
const int POINT_SEPARATION_MAX = 10000;

const int IMAGE_WIDTH = 200;
const int IMAGE_HEIGHT = 350;

const double BRUSH_WIDTH = 2.0;

DlgSettingsSegments::DlgSettingsSegments(MainWindow &mainWindow) :
  DlgSettingsAbstractBase (tr ("Segment Fill"),
                           "DlgSettingsSegments",
                           mainWindow),
  m_scenePreviewActive (nullptr),
  m_scenePreviewInactive (nullptr),
  m_viewPreviewActive (nullptr),  
  m_viewPreviewInactive (nullptr),
  m_modelSegmentsBefore (nullptr),
  m_modelSegmentsAfter (nullptr),
  m_loading (false)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsSegments::DlgSettingsSegments";

  QWidget *subPanel = createSubPanel ();
  finishPanel (subPanel);
}

DlgSettingsSegments::~DlgSettingsSegments()
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsSegments::~DlgSettingsSegments";
}

void DlgSettingsSegments::clearPointsOneScene (GraphicsPoints &points)
{
  QList<GraphicsPoint*>::iterator itrP;
  for (itrP = points.begin(); itrP != points.end(); itrP++) {
    GraphicsPoint *point = *itrP;
    delete point;
  }

  points.clear();
}

void DlgSettingsSegments::createControls (QGridLayout *layout,
                                          int &row)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsSegments::createControls";

  QLabel *labelMinLength = new QLabel(QString ("%1:").arg (tr ("Minimum length (points)")));
  layout->addWidget(labelMinLength, row, 1);

  m_spinMinLength = new QSpinBox;
  m_spinMinLength->setRange (MIN_LENGTH_MIN, MIN_LENGTH_MAX);
  m_spinMinLength->setWhatsThis (tr ("Select a minimum number of points in a segment.\n\n"
                                     "Only segments with more points will be created.\n\n"
                                     "This value should be as large as possible to reduce memory usage. This value has "
                                     "a lower limit"));
  connect (m_spinMinLength, SIGNAL (valueChanged (const QString &)), this, SLOT (slotMinLength (const QString &)));
  layout->addWidget(m_spinMinLength, row++, 2);

  QLabel *labelPointSeparation = new QLabel(QString ("%1:").arg (tr ("Point separation (pixels)")));
  layout->addWidget (labelPointSeparation, row, 1);

  m_spinPointSeparation = new QSpinBox;
  m_spinPointSeparation->setRange (POINT_SEPARATION_MIN, POINT_SEPARATION_MAX);
  m_spinPointSeparation->setWhatsThis (tr ("Select a point separation in pixels.\n\n"
                                           "Successive points added to a segment will be separated by this number of pixels. "
                                           "If Fill Corners is enabled, then additional points will be inserted at corners so some points "
                                           "will be closer.\n\n"
                                           "This value has a lower limit"));
  connect (m_spinPointSeparation, SIGNAL (valueChanged (const QString &)), this, SLOT (slotPointSeparation (const QString &)));
  layout->addWidget (m_spinPointSeparation, row++, 2);

  // The Auto Curve Detection options below are application preferences rather than document data,
  // so they are read from and written to the settings directly
  QLabel *labelAutoCurveMaxGap = new QLabel (QString ("%1:").arg (tr ("Auto Curve Detection maximum gap (pixels)")));
  layout->addWidget (labelAutoCurveMaxGap, row, 1);

  m_spinAutoCurveMaxGap = new QSpinBox;
  m_spinAutoCurveMaxGap->setRange (1, 20);
  m_spinAutoCurveMaxGap->setWhatsThis (tr ("Auto Curve Detection Maximum Gap\n\n"
                                           "Largest distance in pixels between two touching curve pieces that the Auto Curve "
                                           "Detection tool still treats as one curve. Pieces further apart are treated as "
                                           "separate curves."));
  connect (m_spinAutoCurveMaxGap, SIGNAL (valueChanged (const QString &)), this, SLOT (slotAutoCurveMaxGap (const QString &)));
  layout->addWidget (m_spinAutoCurveMaxGap, row++, 2);

  QLabel *labelAutoCurveMaxTurn = new QLabel (QString ("%1:").arg (tr ("Auto Curve Detection maximum turn (degrees)")));
  layout->addWidget (labelAutoCurveMaxTurn, row, 1);

  m_spinAutoCurveMaxTurn = new QSpinBox;
  m_spinAutoCurveMaxTurn->setRange (15, 90);
  m_spinAutoCurveMaxTurn->setWhatsThis (tr ("Auto Curve Detection Maximum Turn\n\n"
                                            "Largest angle in degrees between two consecutive curve pieces before the Auto Curve "
                                            "Detection walk is cut. Small values stop the walk earlier, which keeps the chain from "
                                            "following a grid line or text that touches the curve, at the cost of stopping also at "
                                            "sharp corners of the curve itself."));
  connect (m_spinAutoCurveMaxTurn, SIGNAL (valueChanged (const QString &)), this, SLOT (slotAutoCurveMaxTurn (const QString &)));
  layout->addWidget (m_spinAutoCurveMaxTurn, row++, 2);

  QLabel *labelAutoCurveCenterStrategy = new QLabel (QString ("%1:").arg (tr ("Auto Curve Detection point placement")));
  layout->addWidget (labelAutoCurveCenterStrategy, row, 1);

  m_cmbAutoCurveCenterStrategy = new QComboBox;
  m_cmbAutoCurveCenterStrategy->addItem (tr ("Traced path (upstream behavior)"));
  m_cmbAutoCurveCenterStrategy->addItem (tr ("Center of the curve pixels"));
  m_cmbAutoCurveCenterStrategy->addItem (tr ("Darkness weighted center of the curve pixels"));
  m_cmbAutoCurveCenterStrategy->setWhatsThis (tr ("Auto Curve Detection Point Placement\n\n"
                                                  "Determines where a created point sits inside a thick curve band.\n\n"
                                                  "Traced path keeps the point on the path that the tool traced, which is the "
                                                  "behavior of the original application.\n\n"
                                                  "Center of the curve pixels moves the point to the middle of the curve pixels "
                                                  "in its column, so the points follow the middle of the band.\n\n"
                                                  "Darkness weighted center also moves the point to the middle, but weights the "
                                                  "pixels by darkness, which follows the dark middle of a band with soft edges."));
  connect (m_cmbAutoCurveCenterStrategy, SIGNAL (currentIndexChanged (const QString &)), this, SLOT (slotAutoCurveCenterStrategy (const QString &)));
  layout->addWidget (m_cmbAutoCurveCenterStrategy, row++, 2);

  m_chkAutoCurveFunctionAssumption = new QCheckBox;
  m_chkAutoCurveFunctionAssumption->setWhatsThis (tr ("Auto Curve Detection Function Assumption\n\n"
                                                      "Assume the curve is a function, which means the x coordinate in graph coordinates never "
                                                      "goes backwards. The Auto Curve Detection walk is cut where the assumption is violated, "
                                                      "which keeps the chain from following a vertical grid line. Turn this off for relation "
                                                      "curves that legitimately loop back."));
  connect (m_chkAutoCurveFunctionAssumption, SIGNAL (stateChanged (int)), this, SLOT (slotAutoCurveFunctionAssumption (int)));
  layout->addWidget (m_chkAutoCurveFunctionAssumption, row++, 2);

  QLabel *labelFillCorners = new QLabel (QString ("%1:").arg (tr ("Fill corners")));
  layout->addWidget (labelFillCorners, row, 1);

  m_chkFillCorners = new QCheckBox;
  m_chkFillCorners->setWhatsThis (tr ("Fill corners.\n\n"
                                      "In addition to the points placed at regular intervals, this option causes a point to be "
                                      "placed at each corner. This option can capture important information in piecewise linear graphs, "
                                      "but gradually curving graphs may not benefit from the additional points"));
  connect (m_chkFillCorners, SIGNAL (stateChanged (int)), this, SLOT (slotFillCorners (int)));
  layout->addWidget (m_chkFillCorners, row++, 2);

  QLabel *labelLineWidthActive = new QLabel(QString ("%1:").arg (tr ("Active line width")));
  layout->addWidget (labelLineWidthActive, row, 1);

  m_spinLineWidthActive = new QSpinBox;
  m_spinLineWidthActive->setWhatsThis (tr ("Select a size for the lines drawn along a segment when hovering over the segment"));
  m_spinLineWidthActive->setMinimum(1);
  connect (m_spinLineWidthActive, SIGNAL (valueChanged (int)), this, SLOT (slotLineWidthActive (int)));
  layout->addWidget (m_spinLineWidthActive, row++, 2);

  QLabel *labelLineWidthInactive = new QLabel(QString ("%1:").arg (tr ("Inactive line width")));
  layout->addWidget (labelLineWidthInactive, row, 1);

  m_spinLineWidthInactive = new QSpinBox;
  m_spinLineWidthInactive->setWhatsThis (tr ("Select a size for the lines drawn along a segment when not hovering over the segment"));
  m_spinLineWidthInactive->setMinimum(1);
  connect (m_spinLineWidthInactive, SIGNAL (valueChanged (int)), this, SLOT (slotLineWidthInactive (int)));
  layout->addWidget (m_spinLineWidthInactive, row++, 2);  

  QLabel *labelLineColor = new QLabel(QString ("%1:").arg (tr ("Line color")));
  layout->addWidget (labelLineColor, row, 1);

  m_cmbLineColor = new QComboBox;
  m_cmbLineColor->setWhatsThis (tr ("Select a color for the lines drawn along a segment"));
  populateColorComboWithTransparent (*m_cmbLineColor);
  connect (m_cmbLineColor,
           &QComboBox::textActivated,
           this,
           &DlgSettingsSegments::slotLineColor); // textActivated() ignores code changes
  layout->addWidget (m_cmbLineColor, row++, 2);

  QLabel *labelInactiveOpacity = new QLabel(QString ("%1:").arg (tr ("Inactive opacity")));
  layout->addWidget (labelInactiveOpacity, row, 1);

  m_cmbInactiveOpacity = new QComboBox;
  m_cmbInactiveOpacity->setWhatsThis (tr ("Select an opacity for the inactive segments that are not under the cursor"));
  m_cmbInactiveOpacity->addItem (inactiveOpacityEnumToQString (INACTIVE_OPACITY_0),
                                QVariant (INACTIVE_OPACITY_0));
  m_cmbInactiveOpacity->addItem (inactiveOpacityEnumToQString (INACTIVE_OPACITY_32),
                                QVariant (INACTIVE_OPACITY_32));
  m_cmbInactiveOpacity->addItem (inactiveOpacityEnumToQString (INACTIVE_OPACITY_64),
                                QVariant (INACTIVE_OPACITY_64));
  m_cmbInactiveOpacity->addItem (inactiveOpacityEnumToQString (INACTIVE_OPACITY_96),
                                QVariant (INACTIVE_OPACITY_96));
  m_cmbInactiveOpacity->addItem (inactiveOpacityEnumToQString (INACTIVE_OPACITY_128),
                                QVariant (INACTIVE_OPACITY_128));
  m_cmbInactiveOpacity->addItem (inactiveOpacityEnumToQString (INACTIVE_OPACITY_160),
                                QVariant (INACTIVE_OPACITY_160));
  m_cmbInactiveOpacity->addItem (inactiveOpacityEnumToQString (INACTIVE_OPACITY_192),
                                QVariant (INACTIVE_OPACITY_192));
  m_cmbInactiveOpacity->addItem (inactiveOpacityEnumToQString (INACTIVE_OPACITY_224),
                                QVariant (INACTIVE_OPACITY_224));
  m_cmbInactiveOpacity->addItem (inactiveOpacityEnumToQString (INACTIVE_OPACITY_256),
                                QVariant (INACTIVE_OPACITY_256));
  connect (m_cmbInactiveOpacity,
           &QComboBox::textActivated,
           this,
           &DlgSettingsSegments::slotInactiveOpacity); // textActivated() ignores code changes
  layout->addWidget (m_cmbInactiveOpacity, row++, 2);
}

void DlgSettingsSegments::createOptionalSaveDefault (QHBoxLayout * /* layout */)
{
}

void DlgSettingsSegments::createPreview (QGridLayout *layout,
                                         int &row)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsSegments::createPreview";

  QLabel *labelPreviewActive = new QLabel (tr ("Hover Preview"));
  layout->addWidget (labelPreviewActive, row, 0, 1, 2);

  QLabel *labelPreviewInactive = new QLabel (tr ("Inactive Preview"));
  layout->addWidget (labelPreviewInactive, row++, 2, 1, 2);

  m_scenePreviewActive = new QGraphicsScene (this);
  m_viewPreviewActive = new ViewPreview (m_scenePreviewActive,
                                         ViewPreview::VIEW_ASPECT_RATIO_VARIABLE,
                                         this);
  m_viewPreviewActive->setWhatsThis (tr ("For segments under the cursor, this preview window shows the shortest "
                                         "line that can be segment filled, "
                                         "and the effects of current settings on segments and points generated by segment fill"));
  m_viewPreviewActive->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_viewPreviewActive->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_viewPreviewActive->setMinimumHeight (MINIMUM_PREVIEW_HEIGHT);
  layout->addWidget (m_viewPreviewActive, row, 0, 1, 2);

  m_scenePreviewInactive = new QGraphicsScene (this);
  m_viewPreviewInactive = new ViewPreview (m_scenePreviewInactive,
                                           ViewPreview::VIEW_ASPECT_RATIO_VARIABLE,
                                           this);
  m_viewPreviewInactive->setWhatsThis (tr ("For segments not under the cursor, this preview window shows the shortest "
                                           "line that can be segment filled, "
                                           "and the effects of current settings on segments and points generated by segment fill"));
  m_viewPreviewInactive->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_viewPreviewInactive->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_viewPreviewInactive->setMinimumHeight (MINIMUM_PREVIEW_HEIGHT);
  layout->addWidget (m_viewPreviewInactive, row++, 2, 1, 2);
}

QImage DlgSettingsSegments::createPreviewImage () const
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsSegments::createPreviewImage";

  QImage image (IMAGE_WIDTH,
                IMAGE_HEIGHT,
                QImage::Format_RGB32);
  image.fill (Qt::white);
  QPainter painter (&image);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setPen (QPen (QBrush (Qt::black), BRUSH_WIDTH));

  int margin = IMAGE_WIDTH / 15;
  int yCenter = IMAGE_HEIGHT / 2;
  int yHeight = IMAGE_HEIGHT / 4;
  int x, y, xLast = 0, yLast = 0;
  bool isFirst;

  // Draw sinusoid
  isFirst = true;
  int xStart = margin, xEnd = IMAGE_WIDTH / 2 - margin;
  for (x = xStart; x < xEnd; x++) {
    double s = double (x - xStart) / double (xEnd - xStart);
    int y = qFloor (yCenter - yHeight * qSin (2.0 * M_PI * s));

    if (!isFirst) {
      painter.drawLine (xLast, yLast, x, y);
    }
    isFirst = false;
    xLast = x;
    yLast = y;
  }

  // Draw triangular waveform that looks like sinusoid straightened up into line segments
  isFirst = true;
  xStart = IMAGE_WIDTH / 2 + margin;
  xEnd = IMAGE_WIDTH - margin;
  for (x = xStart; x < xEnd; x++) {
    double s = double (x - xStart) / double (xEnd - xStart);
    if (s <= 0.25) {
      y = qFloor (yCenter - yHeight * (4.0 * s));
    } else if (s < 0.75) {
      y = qFloor (yCenter - yHeight * (1.0 - 4.0 * (s - 0.25)));
    } else {
      y = qFloor (yCenter + yHeight * (1.0 - 4 * (s - 0.75)));
    }

    if (!isFirst) {
      painter.drawLine (xLast, yLast, x, y);
    }
    isFirst = false;
    xLast = x;
    yLast = y;
  }

  return image;
}

QWidget *DlgSettingsSegments::createSubPanel ()
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsSegments::createSubPanel";

  QWidget *subPanel = new QWidget ();
  QGridLayout *layout = new QGridLayout (subPanel);
  subPanel->setLayout (layout);

  layout->setColumnStretch (0, 1); // Empty first column
  layout->setColumnStretch (1, 0); // Labels
  layout->setColumnStretch (2, 0); // User controls
  layout->setColumnStretch (3, 1); // Empty last column

  int row = 0;
  createWhatsThis (layout,
                   m_btnWhatsThis,
                   row++,
                   3);
  createControls(layout, row);
  createPreview (layout, row);
  QPixmap pixmap = QPixmap::fromImage (createPreviewImage());
  addPixmap (*m_scenePreviewActive,
             pixmap);
  pixmap = QPixmap::fromImage (createPreviewImage());
  addPixmap (*m_scenePreviewInactive,
             pixmap);

  return subPanel;
}

void DlgSettingsSegments::handleOk ()
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsSegments::handleOk";

  CmdSettingsSegments *cmd = new CmdSettingsSegments (mainWindow (),
                                                      cmdMediator ().document(),
                                                      *m_modelSegmentsBefore,
                                                      *m_modelSegmentsAfter);
  cmdMediator ().push (cmd);

  // The Auto Curve Detection options are application preferences, so they are stored in the settings
  // and picked up by DocumentModelSegments the next time a document is loaded
  {
    QSettings settings (SETTINGS_ENGAUGE, SETTINGS_DIGITIZER);
    settings.beginGroup (SETTINGS_GROUP_SEGMENTS);

    settings.setValue (SETTINGS_SEGMENTS_MAX_GAP_PIXELS,
                       m_spinAutoCurveMaxGap->value ());
    settings.setValue (SETTINGS_SEGMENTS_MAX_TURN_DEGREES,
                       m_spinAutoCurveMaxTurn->value ());
    settings.setValue (SETTINGS_SEGMENTS_CENTER_STRATEGY,
                       segmentCenterStrategyToString (m_modelSegmentsAfter->centerStrategy ()));
    settings.setValue (SETTINGS_SEGMENTS_FUNCTION_ASSUMPTION,
                       m_chkAutoCurveFunctionAssumption->isChecked ());
    settings.endGroup ();
  }

  hide ();
}

void DlgSettingsSegments::load (CmdMediator &cmdMediator)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsSegments::load";

  // Loading starts here
  m_loading = true;

  setCmdMediator (cmdMediator);

  // Flush old data
  delete m_modelSegmentsBefore;
  delete m_modelSegmentsAfter;

  // Save new data
  m_modelSegmentsBefore = new DocumentModelSegments (cmdMediator.document());
  m_modelSegmentsAfter = new DocumentModelSegments (cmdMediator.document());

  // Sanity checks. Incoming defaults must be acceptable to the local limits
  ENGAUGE_ASSERT (MIN_LENGTH_MIN <= m_modelSegmentsAfter->minLength ());
  ENGAUGE_ASSERT (MIN_LENGTH_MAX >= m_modelSegmentsAfter->minLength ());
  ENGAUGE_ASSERT (POINT_SEPARATION_MIN <= m_modelSegmentsAfter->pointSeparation());
  ENGAUGE_ASSERT (POINT_SEPARATION_MAX >= m_modelSegmentsAfter->pointSeparation());

  // Populate controls
  m_spinPointSeparation->setValue (qFloor (m_modelSegmentsAfter->pointSeparation()));
  m_spinAutoCurveMaxGap->setValue (qFloor (m_modelSegmentsAfter->maxGapPixels ()));
  m_spinAutoCurveMaxTurn->setValue (qFloor (m_modelSegmentsAfter->maxTurnDegrees ()));
  m_cmbAutoCurveCenterStrategy->setCurrentIndex ((int) m_modelSegmentsAfter->centerStrategy ());
  m_chkAutoCurveFunctionAssumption->setChecked (m_modelSegmentsAfter->functionAssumption ());
  m_spinMinLength->setValue (qFloor (m_modelSegmentsAfter->minLength()));
  m_chkFillCorners->setChecked (m_modelSegmentsAfter->fillCorners ());
  m_spinLineWidthActive->setValue (qFloor (m_modelSegmentsAfter->lineWidthActive()));
  m_spinLineWidthInactive->setValue (qFloor (m_modelSegmentsAfter->lineWidthInactive()));  

  int indexLineColor = m_cmbLineColor->findData(QVariant (m_modelSegmentsAfter->lineColor()));
  ENGAUGE_ASSERT (indexLineColor >= 0);
  m_cmbLineColor->setCurrentIndex(indexLineColor);

  int indexInactiveOpacity = m_cmbInactiveOpacity->findData (QVariant (m_modelSegmentsAfter->inactiveOpacity()));
  ENGAUGE_ASSERT (indexInactiveOpacity >= 0);
  m_cmbInactiveOpacity->setCurrentIndex (indexInactiveOpacity);

  // Loading finishes here
  m_loading = false;

  updateControls();
  enableOk (false); // Disable Ok button since there not yet any changes
  updatePreview();
}

void DlgSettingsSegments::setSmallDialogs(bool smallDialogs)
{
  if (!smallDialogs) {
    setMinimumHeight (MINIMUM_HEIGHT);
  }
}

void DlgSettingsSegments::slotAutoCurveMaxGap (const QString &maxGap)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsSegments::slotAutoCurveMaxGap";

  m_modelSegmentsAfter->setMaxGapPixels (maxGap.toDouble ());
}

void DlgSettingsSegments::slotAutoCurveMaxTurn (const QString &maxTurn)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsSegments::slotAutoCurveMaxTurn";

  m_modelSegmentsAfter->setMaxTurnDegrees (maxTurn.toDouble ());
}

void DlgSettingsSegments::slotAutoCurveCenterStrategy (const QString &text)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsSegments::slotAutoCurveCenterStrategy";

  const int index = m_cmbAutoCurveCenterStrategy->currentIndex ();

  m_modelSegmentsAfter->setCenterStrategy ((SegmentCenterStrategy) index);
}

void DlgSettingsSegments::slotAutoCurveFunctionAssumption (int state)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsSegments::slotAutoCurveFunctionAssumption";

  m_modelSegmentsAfter->setFunctionAssumption (state == Qt::Checked);
}

void DlgSettingsSegments::slotFillCorners (int state)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsSegments::slotFillCorner";

  m_modelSegmentsAfter->setFillCorners(state == Qt::Checked);
  updateControls();
  updatePreview();
}

void DlgSettingsSegments::slotInactiveOpacity (const QString &)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsSegments::slotInactiveOpacity";

  InactiveOpacity inactiveOpacity = static_cast<InactiveOpacity> (m_cmbInactiveOpacity->currentData().toInt());
  m_modelSegmentsAfter->setInactiveOpacity(inactiveOpacity);
  updateControls();
  updatePreview();
}

void DlgSettingsSegments::slotLineColor (const QString &)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsSegments::slotLineColor";

  m_modelSegmentsAfter->setLineColor(static_cast<ColorPalette> (m_cmbLineColor->currentData().toInt()));
  updateControls();
  updatePreview();
}

void DlgSettingsSegments::slotLineWidthActive (int lineWidthActive)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsSegments::slotLineWidthActive";

  m_modelSegmentsAfter->setLineWidthActive(lineWidthActive);
  updateControls();
  updatePreview();
}

void DlgSettingsSegments::slotLineWidthInactive (int lineWidthInactive)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsSegments::slotLineWidthInactive";

  m_modelSegmentsAfter->setLineWidthInactive(lineWidthInactive);
  updateControls();
  updatePreview();
}

void DlgSettingsSegments::slotMinLength (const QString &minLength)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsSegments::slotMinLength";

  m_modelSegmentsAfter->setMinLength(minLength.toDouble());
  updateControls();
  updatePreview();
}

void DlgSettingsSegments::slotPointSeparation (const QString &pointSeparation)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsSegments::slotPointSeparation";

  m_modelSegmentsAfter->setPointSeparation(pointSeparation.toDouble());
  updateControls();
  updatePreview();
}

void DlgSettingsSegments::slotWhatsThis ()
{
  QWhatsThis::enterWhatsThisMode();
}

void DlgSettingsSegments::updateControls()
{
  enableOk (true);
}

void DlgSettingsSegments::updatePreview()
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgSettingsSegments::updatePreview"
                              << " loading=" << (m_loading ? "true" : "false");

  updatePreviewOneScene (m_scenePreviewActive,
                         m_segmentsActive,
                         m_pointsActive,
                         HOVER_ACTIVE);
  updatePreviewOneScene (m_scenePreviewInactive,
                         m_segmentsInactive,
                         m_pointsInactive,
                         HOVER_INACTIVE);
}

void DlgSettingsSegments::updatePreviewOneScene (QGraphicsScene *scenePreview,
                                                 QList<Segment*> &segments,
                                                 GraphicsPoints &points,
                                                 HoverState hoverState)
{
  const QString ARBITRARY_IDENTIFIER ("");
  const QColor COLOR (Qt::blue);
  const int RADIUS = 5;
  GeometryWindow *NULL_GEOMETRY_WINDOW = nullptr;
  const bool NO_DIALOG = false; // If true then dueling modal dialogs will trigger infinite loops in QSpinBox up/down

  if (!m_loading) {

    SegmentFactory segmentFactory (*scenePreview,
                                   mainWindow().isGnuplot());

    clearPointsOneScene (points);
    segmentFactory.clearSegments (segments);

    // Create new segments
    segmentFactory.makeSegments (createPreviewImage(),
                                 *m_modelSegmentsAfter,
                                 segments,
                                 NO_DIALOG);

    // Make the segment visible
    QList<Segment*>::iterator itrS;
    for (itrS = segments.begin(); itrS != segments.end(); itrS++) {
      Segment *segment = *itrS;

      segment->slotHover (hoverState == HOVER_ACTIVE);
      segment->lockHoverState ();
    }

    // Create some points
    PointStyle pointStyle (POINT_SHAPE_CROSS,
                           RADIUS,
                           qFloor (BRUSH_WIDTH),
                           COLOR_PALETTE_BLUE);
    QPolygonF polygon = pointStyle.polygon();
    QList<QPoint> pointsFill = segmentFactory.fillPoints (*m_modelSegmentsAfter,
                                                          segments);

    QList<QPoint>::iterator itrP;
    for (itrP = pointsFill.begin(); itrP != pointsFill.end(); itrP++) {
      QPoint pos = *itrP;
      GraphicsPoint *graphicsPoint = new GraphicsPoint (*scenePreview,
                                                        ARBITRARY_IDENTIFIER,
                                                        pos,
                                                        COLOR,
                                                        polygon,
                                                        BRUSH_WIDTH,
                                                        NULL_GEOMETRY_WINDOW);

      points.push_back (graphicsPoint);
    }
  }
}
