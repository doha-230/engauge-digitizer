/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "BatchProcessor.h"
#include "CallbackCollectAxisPoints.h"
#include "CmdMediator.h"
#include "Curve.h"
#include "Document.h"
#include "ExportToFile.h"
#include "Logger.h"
#include "MainWindow.h"
#include "Point.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QtGlobal>
#include <iostream>
#include "functor.h"

BatchProcessor::BatchProcessor ()
{
}

void BatchProcessor::applyTemplate (MainWindow &mainWindow,
                                    const QString &templateFile)
{
  // The template knowledge lives in a normal document, so it is loaded like any other document and
  // its models and axis points are copied onto the current document
  CmdMediator cmdTemplate (mainWindow,
                           templateFile);
  if (!cmdTemplate.successfulRead ()) {
    LOG4CPP_ERROR_S ((*mainCat)) << "BatchProcessor::applyTemplate cannot read template"
                                 << templateFile.toLatin1().data();
    return;
  }

  Document &document = mainWindow.cmdMediator()->document();

  // Digitizing knowledge: models
  document.setModelCoords (cmdTemplate.document().modelCoords ());
  document.setModelColorFilter (cmdTemplate.document().modelColorFilter ());
  document.setModelGridRemoval (cmdTemplate.document().modelGridRemoval ());
  document.setModelSegments (cmdTemplate.document().modelSegments ());
  document.setModelExport (cmdTemplate.document().modelExport ());
  document.setModelPointMatch (cmdTemplate.document().modelPointMatch ());

  // Axis points: the template graph coordinates are copied onto the axis points of the current
  // document, in ordinal order, since the coordinates define the coordinate system
  QList<Point> axisPointsTemplate;
  {
    CallbackCollectAxisPoints ftor (axisPointsTemplate);
    Functor2wRet<const QString &, const Point &, CallbackSearchReturn> ftorWithCallback = functor_ret (ftor,
                                                                                                       &CallbackCollectAxisPoints::callback);
    cmdTemplate.document().curveAxes ().iterateThroughCurvePoints (ftorWithCallback);
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

    // The template graph coordinates define the coordinate system. The screen position of the
    // template is copied as well, so the axis point stays where the user digitized it in the template
    // image, which is the position that lines up with a new image of the same layout.
    pointCurrent.setPosGraph (pointTemplate.posGraph ());
    pointCurrent.setPosScreen (pointTemplate.posScreen ());
  }
}

bool BatchProcessor::process (MainWindow &mainWindow,
                              const QString &templateFile,
                              const QString &outputDirectory,
                              bool continueAfterError,
                              const QStringList &inputFiles)
{
  LOG4CPP_INFO_S ((*mainCat)) << "BatchProcessor::process"
                              << " template=" << templateFile.toLatin1().data()
                              << " outputDirectory=" << outputDirectory.toLatin1().data()
                              << " files=" << inputFiles.count ();

  m_summary.clear ();
  int failures = 0;

  for (int index = 0; index < inputFiles.count (); index++) {

    const QString inputFile = inputFiles.at (index);
    const QFileInfo inputInfo (inputFile);

    QString result;
    if (!QFile::exists (inputFile)) {
      result = "missing input file";
    } else {
      try {
        // Import, apply the template knowledge, export
        mainWindow.fileImport (inputFile,
                               MainWindow::IMPORT_TYPE_SIMPLE);
        applyTemplate (mainWindow,
                       templateFile);

        const QString outputBase = outputDirectory.isEmpty () ?
                                   inputInfo.absolutePath () :
                                   outputDirectory;
        QDir ().mkpath (outputBase);

        const QString outputFile = QDir (outputBase).filePath (inputInfo.completeBaseName () + ".csv");

        mainWindow.fileExport (outputFile,
                               ExportToFile ());

        result = QFile::exists (outputFile) ? "ok" : "export failed";
      } catch (...) {
        result = "exception";
      }
    }

    const bool ok = (result == "ok");
    m_summary << QString ("%1,%2,%3")
                     .arg (inputInfo.fileName ())
                     .arg (ok ? "ok" : "failed")
                     .arg (result);

    if (!ok) {
      failures++;
      if (!continueAfterError) {
        break;
      }
    }
  }

  // Summary file next to the results
  const QString summaryFile = (outputDirectory.isEmpty () ?
                               QDir::currentPath () :
                               outputDirectory) + "/batch_summary.csv";
  QFile file (summaryFile);
  if (file.open (QIODevice::WriteOnly | QIODevice::Text)) {
    QTextStream str (&file);
    str << "file,result,detail\n";
    for (int index = 0; index < m_summary.count (); index++) {
      str << m_summary.at (index) << "\n";
    }
  }

  for (int index = 0; index < m_summary.count (); index++) {
    std::cout << m_summary.at (index).toLatin1().data () << std::endl;
  }
  std::cout << "batch summary: " << (inputFiles.count () - failures) << " ok, "
            << failures << " failed" << std::endl;

  return failures == 0;
}

const QStringList &BatchProcessor::summary () const
{
  return m_summary;
}
