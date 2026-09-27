/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "DigitizeStateAutoCurve.h"

#include "CmdAddPointsGraph.h"
#include "CmdMediator.h"
#include "Document.h"
#include "DocumentModelSegments.h"
#include "DigitizeStateContext.h"
#include "EngaugeAssert.h"
#include "GraphicsScene.h"
#include "Logger.h"
#include "MainWindow.h"
#include "OrdinalGenerator.h"
#include "PointOrigin.h"
#include "QualityReport.h"
#include "Segment.h"
#include "SegmentChain.h"
#include "SegmentCenter.h"
#include "SegmentFactory.h"
#include "Transformation.h"
#include <QApplication>
#include <QCursor>
#include <QImage>
#include <QMessageBox>
#include <QPixmap>

// The chain stops growing when two consecutive pieces meet at a bigger angle than this. A curve keeps
// its direction where two scanned pieces touch, while a grid line crosses the curve at a large angle
const double DEFAULT_MAX_TURN_DEGREES = 45.0;

// Hard safety limit on the number of points one detection may create, so a wrong click on a full
// image of touching lines cannot freeze the application
const int MAX_POINTS_PER_DETECTION = 5000;

DigitizeStateAutoCurve::DigitizeStateAutoCurve(DigitizeStateContext &context) :
  QObject (),
  DigitizeStateAbstractBase (context),
  m_cmdMediator (nullptr)
{
}

DigitizeStateAutoCurve::~DigitizeStateAutoCurve ()
{
}

QString DigitizeStateAutoCurve::activeCurve () const
{
  return context().mainWindow().selectedGraphCurve();
}

void DigitizeStateAutoCurve::begin (CmdMediator *cmdMediator,
                                    DigitizeState previousState)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DigitizeStateAutoCurve::begin"
                              << " previous=" << digitizeStateAsString (previousState).toLatin1().data();

  m_cmdMediator = cmdMediator;

  rebuildSegments (cmdMediator);

  context().mainWindow().updateAfterCommand();
}

bool DigitizeStateAutoCurve::canPaste (const Transformation &transformation,
                                       const QSize &viewSize) const
{
  // Same policy as the Segment Fill state: pasting points is a select-state operation
  Q_UNUSED (transformation);
  Q_UNUSED (viewSize);

  return false;
}

QCursor DigitizeStateAutoCurve::cursor (CmdMediator * /* cmdMediator */) const
{
  return QCursor (Qt::CrossCursor);
}

void DigitizeStateAutoCurve::end ()
{
  LOG4CPP_INFO_S ((*mainCat)) << "DigitizeStateAutoCurve::end";
}

bool DigitizeStateAutoCurve::guidelinesAreSelectable () const
{
  return false;
}

void DigitizeStateAutoCurve::handleContextMenuEventAxis (CmdMediator * /* cmdMediator */,
                                                         const QString & /* pointIdentifier */)
{
}

void DigitizeStateAutoCurve::handleContextMenuEventGraph (CmdMediator * /* cmdMediator */,
                                                          const QStringList & /* pointIdentifiers */)
{
}

void DigitizeStateAutoCurve::handleCurveChange (CmdMediator *cmdMediator)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DigitizeStateAutoCurve::handleCurveChange";

  rebuildSegments (cmdMediator);
}

void DigitizeStateAutoCurve::handleKeyPress (CmdMediator *cmdMediator,
                                             Qt::Key key,
                                             bool atLeastOneSelectedItem)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DigitizeStateAutoCurve::handleKeyPress";

  handleKeyPressArrow (cmdMediator,
                       key,
                       atLeastOneSelectedItem);
}

void DigitizeStateAutoCurve::handleMouseMove (CmdMediator * /* cmdMediator */,
                                              QPointF /* posScreen */)
{
}

void DigitizeStateAutoCurve::handleMousePress (CmdMediator * /* cmdMediator */,
                                               QPointF /* posScreen */)
{
}

void DigitizeStateAutoCurve::handleMouseRelease (CmdMediator * /* cmdMediator */,
                                                 QPointF /* posScreen */)
{
}

QString DigitizeStateAutoCurve::state () const
{
  return "DigitizeStateAutoCurve";
}

void DigitizeStateAutoCurve::updateAfterPointAddition ()
{
}

void DigitizeStateAutoCurve::updateModelDigitizeCurve (CmdMediator * /* cmdMediator */,
                                                       const DocumentModelDigitizeCurve & /* modelDigitizeCurve */)
{
}

void DigitizeStateAutoCurve::updateModelSegments (const DocumentModelSegments & /* modelSegments */)
{
  // The segment lines visible in this mode follow the settings like in the Segment Fill mode, and the
  // chain links are rebuilt on the next curve change or mode entry
  LOG4CPP_INFO_S ((*mainCat)) << "DigitizeStateAutoCurve::updateModelSegments";
}

void DigitizeStateAutoCurve::rebuildSegments (CmdMediator *cmdMediator)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DigitizeStateAutoCurve::rebuildSegments";

  QImage img = context().mainWindow().imageFiltered();

  GraphicsScene &scene = context().mainWindow().scene();
  SegmentFactory segmentFactory (dynamic_cast<QGraphicsScene &> (scene),
                                 context().isGnuplot());

  segmentFactory.clearSegments (m_segments);

  segmentFactory.makeSegments (img,
                               cmdMediator->document().modelSegments(),
                               m_segments);

  // Connect the click signal of every new segment
  QList<Segment*>::iterator itr;
  for (itr = m_segments.begin(); itr != m_segments.end(); itr++) {

    Segment *segment = *itr;

    disconnect (segment, SIGNAL (signalMouseClickOnSegment (QPointF)), this, SLOT (slotMouseClickOnSegment (QPointF)));
    connect (segment, SIGNAL (signalMouseClickOnSegment (QPointF)), this, SLOT (slotMouseClickOnSegment (QPointF)));
  }

  m_links = segmentFactory.chainLinks (m_segments,
                                       cmdMediator->document().modelSegments().maxGapPixels ());
}

QList<Segment*> DigitizeStateAutoCurve::applyFunctionAssumption (const QList<Segment*> &chain,
                                                                 CmdMediator *cmdMediator)
{
  // Walk the chain in graph coordinates and cut at the first x reversal. The pieces before the cut
  // are the detection result, which keeps a vertical grid excursion from filling the whole image.
  QList<Segment*> result;
  if (chain.isEmpty ()) {
    return result;
  }

  const Transformation &transformation = context().mainWindow().transformation();
  if (!transformation.transformIsDefined ()) {
    return chain; // No graph coordinates yet, so the gate cannot be applied
  }

  result << chain.at (0);

  double xLastGraph = 0.0;
  bool haveLast = false;

  for (int index = 0; index < chain.count (); index++) {

    Segment *segment = chain.at (index);
    ENGAUGE_CHECK_PTR (segment);

    if (segment->lineCount () == 0) {
      continue;
    }

    QPointF posGraph;
    transformation.transformScreenToRawGraph (segment->lastPoint (),
                                              posGraph);

    if (haveLast && (posGraph.x () < xLastGraph)) {
      // x went backwards: cut here and keep what came before
      LOG4CPP_INFO_S ((*mainCat)) << "DigitizeStateAutoCurve::applyFunctionAssumption cut"
                                  << " piece=" << index;
      break;
    }

    xLastGraph = posGraph.x ();
    haveLast = true;

    if (!result.contains (segment)) {
      result << segment;
    }
  }

  return result;
}

void DigitizeStateAutoCurve::createPointsAlongChain (const QList<Segment*> &chain)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DigitizeStateAutoCurve::createPointsAlongChain"
                              << " pieces=" << chain.count ();

  if (chain.isEmpty ()) {
    return;
  }

  const DocumentModelSegments &modelSegments = m_cmdMediator->document().modelSegments();

  // Total expected point count, for the safety gate
  double totalLength = 0.0;
  for (int index = 0; index < chain.count (); index++) {
    totalLength += chain.at (index)->length ();
  }
  const double expectedPoints = totalLength / qMax (1.0, modelSegments.pointSeparation ());

  if (expectedPoints > MAX_POINTS_PER_DETECTION) {
    QMessageBox::warning (&context().mainWindow(),
                          context().mainWindow().selectedGraphCurve(),
                          QObject::tr ("The chain of touching curve pieces is too long: %1 points would be created, "
                                       "which is above the limit of %2. Use the Segment Fill tool on smaller pieces instead.")
                          .arg ((int) expectedPoints)
                          .arg (MAX_POINTS_PER_DETECTION));
    return;
  }

  // Points along every piece, in chain order
  GraphicsScene &scene = context().mainWindow().scene();
  SegmentFactory segmentFactory (dynamic_cast<QGraphicsScene &> (scene),
                                 context().isGnuplot());

  QList<QPoint> points = segmentFactory.fillPoints (modelSegments,
                                                    chain);

  // Band centering, same as in the Segment Fill mode. The default strategy keeps the traced path.
  points = SegmentCenter::centerPoints (points,
                                        context ().mainWindow ().imageFiltered (),
                                        modelSegments.centerStrategy ());
  if (points.isEmpty ()) {
    return;
  }

  // One ordinal per point
  OrdinalGenerator ordinalGenerator;
  Document &document = m_cmdMediator->document ();
  const Transformation &transformation = context().mainWindow().transformation();
  QList<double> ordinals;
  QList<QPoint>::iterator itr;
  for (itr = points.begin(); itr != points.end(); itr++) {

    QPoint point = *itr;
    ordinals << ordinalGenerator.generateCurvePointOrdinal(document,
                                                           transformation,
                                                           point,
                                                           activeCurve ());
  }

  // One command, so the whole detection is one undo step
  QUndoCommand *cmd = new CmdAddPointsGraph (context ().mainWindow(),
                                             document,
                                             context ().mainWindow().selectedGraphCurve(),
                                             points,
                                             ordinals,
                                             POINT_ORIGIN_SEGMENT_FILL);
  context().appendNewCmd(m_cmdMediator,
                         cmd);

  // Immediate quality feedback on the points that were just created, as information rather than a
  // blocker: the user decides what to do with the candidates
  QualityReport qualityReport;
  qualityReport.analyze (m_cmdMediator->document (),
                         transformation);
  const QString summaryText = QObject::tr ("Auto-detected: %1 points, %2 review candidates")
                              .arg (points.count ())
                              .arg (qualityReport.issues ().count ());
  context().mainWindow().showTemporaryMessage (summaryText);

  LOG4CPP_INFO_S ((*mainCat)) << "DigitizeStateAutoCurve::createPointsAlongChain done"
                              << " points=" << points.count ();
}

void DigitizeStateAutoCurve::slotMouseClickOnSegment (QPointF posSegmentStart)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DigitizeStateAutoCurve::slotMouseClickOnSegment";

  if (m_cmdMediator == nullptr) {
    return;
  }

  // Find the clicked segment by its first point, exactly like the Segment Fill state does
  Segment *segmentClicked = nullptr;
  for (int index = 0; index < m_segments.count (); index++) {

    Segment *segment = m_segments.at (index);
    ENGAUGE_CHECK_PTR (segment);

    if ((segment->lineCount () > 0) && (segment->firstPoint () == posSegmentStart)) {
      segmentClicked = segment;
      break;
    }
  }

  if (segmentClicked == nullptr) {
    LOG4CPP_ERROR_S ((*mainCat)) << "DigitizeStateAutoCurve::slotMouseClickOnSegment no segment";
    return;
  }

  // Follow the chain of touching pieces. The direction gate inside SegmentChain stops the walk at a
  // sharp angle, which is where a grid line or a glyph branches off.
  const DocumentModelSegments &modelSegments = m_cmdMediator->document().modelSegments();
  SegmentChain chain (m_links,
                      modelSegments.maxGapPixels (),
                      modelSegments.maxTurnDegrees ());
  QList<Segment*> pieces = chain.chainFrom (segmentClicked);

  if (pieces.isEmpty ()) {
    return;
  }

  // Optional function assumption: cut the chain where x goes backwards in graph coordinates
  if (modelSegments.functionAssumption ()) {
    pieces = applyFunctionAssumption (pieces,
                                      m_cmdMediator);
  }

  createPointsAlongChain (pieces);
}
