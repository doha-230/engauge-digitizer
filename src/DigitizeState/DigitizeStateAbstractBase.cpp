/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "CmdMediator.h"
#include "CmdMoveBy.h"
#include "DigitizeStateAbstractBase.h"
#include "DigitizeStateContext.h"
#include "Document.h"
#include "EngaugeAssert.h"
#include "GraphicsItemsExtractor.h"
#include "GraphicsItemType.h"
#include "GraphicsScene.h"
#include "GraphicsView.h"
#include "Logger.h"
#include "MainWindow.h"
#include "MainWindowModel.h"
#include "MimePointsDetector.h"
#include <QCursor>
#include <QGraphicsScene>
#include <QImage>
#include <QSize>
#include <QTimer>
#include "QtToString.h"
#include "Transformation.h"

const QString MOVE_TEXT_DOWN (QObject::tr ("Move down"));
const QString MOVE_TEXT_LEFT (QObject::tr ("Move left"));
const QString MOVE_TEXT_RIGHT (QObject::tr ("Move right"));
const QString MOVE_TEXT_UP (QObject::tr ("Move up"));

DigitizeStateAbstractBase::DigitizeStateAbstractBase(DigitizeStateContext &context) :
  m_context (context)
{
}

DigitizeStateAbstractBase::~DigitizeStateAbstractBase()
{
}

bool DigitizeStateAbstractBase::canPasteProtected (const Transformation &transformation,
                                                   const QSize &viewSize) const
{
  MimePointsDetector mimePointsDetector;

  return mimePointsDetector.isMimePointsData (transformation,
                                              viewSize);
}

DigitizeStateContext &DigitizeStateAbstractBase::context()
{
  return m_context;
}

const DigitizeStateContext &DigitizeStateAbstractBase::context() const
{
  return m_context;
}

void DigitizeStateAbstractBase::handleKeyPressArrow (CmdMediator *cmdMediator,
                                                     Qt::Key key,
                                                     bool atLeastOneSelectedItem,
                                                     Qt::KeyboardModifiers modifiers)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DigitizeStateAbstractBase::handleKeyPressArrow"
                              << " key=" << QKeySequence (key).toString ().toLatin1 ().data ()
                              << " modifiers=" << (int) modifiers;

  if (atLeastOneSelectedItem) {

    if (key == Qt::Key_Down ||
      key == Qt::Key_Up ||
      key == Qt::Key_Left ||
      key == Qt::Key_Right) {

      keyPressArrow (cmdMediator,
                     key,
                     modifiers);

    }
  }
}

void DigitizeStateAbstractBase::keyPressArrow (CmdMediator *cmdMediator,
                                               Qt::Key key,
                                               Qt::KeyboardModifiers modifiers)
{
  // Step size in unzoomed screen pixels: one pixel for a plain arrow key (the upstream behavior),
  // ten pixels for Shift and fifty for Control, for the fast and the very fast lane
  double step = 1.0;
  if (modifiers & Qt::ShiftModifier) {
    step = 10.0;
  } else if (modifiers & Qt::ControlModifier) {
    step = 50.0;
  }

  QPointF deltaScreen;
  QString moveText;
  switch (key) {
    case Qt::Key_Down:
      deltaScreen = QPointF (0, step * zoomedToUnzoomedScreenY ());
      moveText = moveTextDown();
      break;

    case Qt::Key_Left:
      deltaScreen = QPointF (-step * zoomedToUnzoomedScreenX (), 0);
      moveText = moveTextLeft();
      break;

    case Qt::Key_Right:
      deltaScreen = QPointF (step * zoomedToUnzoomedScreenX (), 0);
      moveText = moveTextRight();
      break;

    case Qt::Key_Up:
      deltaScreen = QPointF (0, -step * zoomedToUnzoomedScreenY ());
      moveText = moveTextUp();
      break;

    default:
      ENGAUGE_ASSERT (false);
  }

  // Create command to move points
  GraphicsItemsExtractor graphicsItemsExtractor;
  const QList<QGraphicsItem*> &items  = context().mainWindow().scene ().selectedItems();
  CmdMoveBy *cmd = new CmdMoveBy (context().mainWindow(),
                                  cmdMediator->document(),
                                  deltaScreen,
                                  moveText,
                                  graphicsItemsExtractor.selectedPointIdentifiers (items));
  context().appendNewCmd (cmdMediator,
                          cmd);
}

QString DigitizeStateAbstractBase::moveTextDown () const
{
  return MOVE_TEXT_DOWN;
}

QString DigitizeStateAbstractBase::moveTextLeft () const
{
  return MOVE_TEXT_LEFT;
}

QString DigitizeStateAbstractBase::moveTextRight () const
{
  return MOVE_TEXT_RIGHT;
}

QString DigitizeStateAbstractBase::moveTextUp () const
{
  return MOVE_TEXT_UP;
}

void DigitizeStateAbstractBase::setCursor(CmdMediator *cmdMediator)
{
  LOG4CPP_DEBUG_S ((*mainCat)) << "DigitizeStateAbstractBase::setCursor";

  // Note that we are setting the QGraphicsView cursor and NOT the QApplication override cursor
  m_context.view ().setCursor (cursor (cmdMediator));
}

double DigitizeStateAbstractBase::zoomedToUnzoomedScreenX () const
{
  double m11 = context().mainWindow ().view ().transform().m11 ();
  return 1.0 / m11;
}

double DigitizeStateAbstractBase::zoomedToUnzoomedScreenY () const
{
  double m22 = context().mainWindow ().view ().transform().m22 ();
  return 1.0 / m22;
}
