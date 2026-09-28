/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "DiagnosticsCollector.h"
#include "CmdMediator.h"
#include "Document.h"
#include "Logger.h"
#include "MainWindow.h"
#include "Version.h"
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QRegularExpression>
#include <QScreen>
#include <QTextStream>
#include <QtGlobal>

namespace {

// The log can be huge, and the interesting part of a problem report is what happened last. Ten
// thousand lines is a compromise between completeness and clipboard size
const int LOG_TAIL_LINE_LIMIT = 10000;

QString logTail ()
{
  // The log file name is built by main.cpp from the application directory or the home directory,
  // whichever it could write to. Rebuild the same candidate list instead of guessing
  const QString logName = ".engauge.log";
  QStringList candidates;
  candidates << QDir (QCoreApplication::applicationDirPath ()).filePath (logName)
             << QDir::home ().filePath (logName);

  for (int index = 0; index < candidates.count (); index++) {
    QFile file (candidates.at (index));
    if (!file.exists ()) {
      continue;
    }
    if (file.open (QIODevice::ReadOnly | QIODevice::Text)) {
      const QStringList allLines = QString::fromUtf8 (file.readAll ()).split ('\n');
      const int first = qMax (0, allLines.count () - LOG_TAIL_LINE_LIMIT);
      QStringList tail = allLines.mid (first);
      while (!tail.isEmpty () && tail.last ().isEmpty ()) {
        tail.removeLast ();
      }
      return tail.join ('\n');
    }
  }

  return QObject::tr ("(log file not found)");
}

}

QString DiagnosticsCollector::collect (MainWindow &mainWindow)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DiagnosticsCollector::collect";

  QString text;
  QTextStream str (&text);

  str << "Engauge Digitizer diagnostics\n";
  str << "=============================\n\n";

  str << "Version: " << VERSION_NUMBER << "\n";
  str << "Qt runtime: " << qVersion () << "\n";
  str << "Build date: " << __DATE__ << "\n";
  str << "Generated: " << QDateTime::currentDateTime ().toString (Qt::ISODate) << "\n\n";

  str << "Operating system: " << QSysInfo::prettyProductName () << "\n";
  const QScreen *screen = QGuiApplication::primaryScreen ();
  if (screen) {
    str << "Screen: " << screen->geometry ().width () << "x" << screen->geometry ().height ()
        << " at " << screen->devicePixelRatio () << "x scale\n";
  }
  str << "\n";

  const QString documentFile = mainWindow.currentDocumentPath ();
  str << "Document: " << (documentFile.isEmpty () ? "(none)" : documentFile) << "\n";
  if (!documentFile.isEmpty ()) {
    const Document &document = mainWindow.cmdMediator ()->document ();
    str << "Curves: " << document.curvesGraphs ().curvesGraphsNames ().join (", ") << "\n";
  }
  str << "\n";

  str << "Application log (tail):\n-----------------------\n";
  str << logTail () << "\n";

  return text;
}
