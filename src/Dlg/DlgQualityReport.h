/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#ifndef DLG_QUALITY_REPORT_H
#define DLG_QUALITY_REPORT_H

#include <QDialog>
#include <QString>
#include <QList>
#include "QualityReport.h"

class MainWindow;
class QLabel;
class QTableWidget;
class QualityReport;

/// Shows the points of the current document that deserve a second look, and lets the list be saved
/// as a comma separated file so it can be attached to notes or to a review.
class DlgQualityReport : public QDialog
{
  Q_OBJECT
public:
  /// Single constructor
  DlgQualityReport (MainWindow &mainWindow,
                    const QualityReport &report);

  virtual ~DlgQualityReport ();

signals:
  /// The user double clicked a row, so the main window can center on and select that point
  void signalPointSelected (const QString &curveName,
                            const QString &pointIdentifier);

private slots:
  void slotCellDoubleClicked (int row, int column);
  void slotSaveCsv ();

private:
  void createWidgets (const QualityReport &report);
  void populateTable (const QualityReport &report);

  MainWindow &m_mainWindow;
  QList<QualityIssue> m_issues;
  QString m_reportCsv;
  QLabel *m_lblSummary;
  QTableWidget *m_table;
};

#endif // DLG_QUALITY_REPORT_H
