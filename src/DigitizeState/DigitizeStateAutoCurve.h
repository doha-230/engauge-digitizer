/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#ifndef DIGITIZE_STATE_AUTO_CURVE_H
#define DIGITIZE_STATE_AUTO_CURVE_H

#include "DigitizeStateAbstractBase.h"
#include <QList>
#include <QObject>

class CmdMediator;
class DocumentModelSegments;
class Segment;
struct SegmentChainLink;

/// Digitizing state for the Auto Curve Detection mode: one click on any piece of a curve creates
/// Points along the whole curve, following the chain of touching segment pieces, so gaps where the
/// scanner dropped branch pixels do not have to be filled by hand.
///
/// Premise (documented): curves do not branch. Every piece then has at most one neighbor per side.
///
/// Sanity gates keep the chain from following things that are not the curve:
/// - a direction gate cuts the chain where two consecutive pieces meet at too sharp an angle, which
///   is what a grid line crossing the curve looks like;
/// - an optional function assumption cuts the chain where the x coordinate in graph coordinates goes
///   backwards, which is what following a vertical grid line looks like.
///
/// The created points are ordinary graph points with the automated origin tag, so they can be moved,
/// edited and deleted like any other point, and Delete Automated Points removes them as a group.
class DigitizeStateAutoCurve : public QObject, public DigitizeStateAbstractBase
{
  Q_OBJECT;

public:
  /// Single constructor.
  DigitizeStateAutoCurve(DigitizeStateContext &context);
  virtual ~DigitizeStateAutoCurve();

  virtual QString activeCurve () const;
  virtual void begin(CmdMediator *cmdMediator,
                     DigitizeState previousState);
  virtual bool canPaste (const Transformation &transformation,
                         const QSize &viewSize) const;
  virtual QCursor cursor (CmdMediator *cmdMediator) const;
  virtual void end();
  virtual bool guidelinesAreSelectable () const;
  virtual void handleContextMenuEventAxis (CmdMediator *cmdMediator,
                                           const QString &pointIdentifier);
  virtual void handleContextMenuEventGraph (CmdMediator *cmdMediator,
                                            const QStringList &pointIdentifiers);
  virtual void handleCurveChange(CmdMediator *cmdMediator);
  virtual void handleKeyPress (CmdMediator *cmdMediator,
                               Qt::Key key,
                               bool atLeastOneSelectedItem);
  virtual void handleMouseMove (CmdMediator *cmdMediator,
                                QPointF posScreen);
  virtual void handleMousePress (CmdMediator *cmdMediator,
                                 QPointF posScreen);
  virtual void handleMouseRelease (CmdMediator *cmdMediator,
                                   QPointF posScreen);
  virtual QString state() const;
  virtual void updateAfterPointAddition ();
  virtual void updateModelDigitizeCurve (CmdMediator *cmdMediator,
                                         const DocumentModelDigitizeCurve &modelDigitizeCurve);
  virtual void updateModelSegments(const DocumentModelSegments &modelSegments);

public slots:
  /// Receive signal from Segment that has been clicked on. The CmdMediator from the begin method will be used
  void slotMouseClickOnSegment(QPointF);

private:
  DigitizeStateAutoCurve();

  /// Rebuild the segment list and the chain links from the filtered image
  void rebuildSegments (CmdMediator *cmdMediator);

  /// Create the points along the chain of segments, as one undoable command
  void createPointsAlongChain (const QList<Segment*> &chain);

  /// Cut the chain at the piece boundary where the function assumption (no x going backwards in graph
  /// coordinates) is violated. Returns the longest surviving run of pieces.
  QList<Segment*> applyFunctionAssumption (const QList<Segment*> &chain,
                                           CmdMediator *cmdMediator);

  QList<Segment*> m_segments;
  QList<SegmentChainLink> m_links;
  CmdMediator *m_cmdMediator;
};

#endif // DIGITIZE_STATE_AUTO_CURVE_H
