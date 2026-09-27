/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#ifndef BATCH_PROCESSOR_H
#define BATCH_PROCESSOR_H

#include <QString>
#include <QStringList>

class MainWindow;

/// Batch processing from a template document (the -batchtemplate command line option).
///
/// A template is a normal Engauge document that holds the digitizing knowledge: the axis point graph
/// coordinates that define the coordinate system, the color filter settings, the grid removal settings
/// and the export format. For every input image the processor imports the image, copies that knowledge
/// from the template, exports the data as a comma separated file next to the input image, and moves on
/// to the next image.
///
/// Everything is local file access, which is what a closed network deployment requires. The processor
/// writes a summary file and a summary line to the standard output, and reports success or failure
/// through its exit code.
class BatchProcessor
{
public:
  /// Single constructor
  BatchProcessor ();

  /// Process the files. templateFile is a .dig document, outputDirectory receives the exported files
  /// when it is not empty (otherwise the files land next to their input images), and continueAfterError
  /// decides whether the first failed file stops the batch.
  bool process (MainWindow &mainWindow,
                const QString &templateFile,
                const QString &outputDirectory,
                bool continueAfterError,
                const QStringList &inputFiles);

  /// Text of the summary that was written, one line per processed file plus a result line
  const QStringList &summary () const;

  /// Apply the template knowledge to the current document
  void applyTemplate (MainWindow &mainWindow,
                      const QString &templateFile);

  QStringList m_summary;
};

#endif // BATCH_PROCESSOR_H
