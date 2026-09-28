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
#include "TemplateManager.h"
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
        TemplateManager::applyTemplateToCurrentDocument (mainWindow,
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
