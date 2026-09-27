/******************************************************************************************************
 * (C) 2014 markummitchell@github.com. This file is part of Engauge Digitizer, which is released      *
 * under GNU General Public License version 2 (GPLv2) or (at your option) any later version. See file *
 * LICENSE or go to gnu.org/licenses for details. Distribution requires prior written permission.     *
 ******************************************************************************************************/

#include "DlgQualityReport.h"
#include "Logger.h"
#include "MainWindow.h"
#include "QualityReport.h"
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextStream>
#include <QVBoxLayout>

DlgQualityReport::DlgQualityReport (MainWindow &mainWindow,
                                    const QualityReport &report) :
  QDialog (&mainWindow),
  m_mainWindow (mainWindow),
  m_lblSummary (nullptr),
  m_table (nullptr)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgQualityReport::DlgQualityReport";

  setWindowTitle (tr ("Digitizing Quality Report"));

  // Kept so that Save As CSV writes exactly what the table shows
  m_reportCsv = report.toCsv ();

  createWidgets (report);
  populateTable (report);

  resize (700, 400);
}

DlgQualityReport::~DlgQualityReport ()
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgQualityReport::~DlgQualityReport";
}

void DlgQualityReport::createWidgets (const QualityReport &report)
{
  QVBoxLayout *layout = new QVBoxLayout (this);

  m_lblSummary = new QLabel (report.summary ());
  layout->addWidget (m_lblSummary);

  QLabel *lblExplanation = new QLabel (tr ("These points are candidates for review, not errors: a curve that is "
                                           "digitized as a relation may legitimately repeat an x value. The points "
                                           "are listed so that a curve does not have to be checked by eye."));
  lblExplanation->setWordWrap (true);
  layout->addWidget (lblExplanation);

  m_table = new QTableWidget (this);
  m_table->setColumnCount (6);
  QStringList headers;
  headers << tr ("Curve") << tr ("Point") << tr ("Issue") << tr ("X") << tr ("Y") << tr ("Detail");
  m_table->setHorizontalHeaderLabels (headers);
  m_table->setEditTriggers (QAbstractItemView::NoEditTriggers);
  m_table->setSelectionBehavior (QAbstractItemView::SelectRows);
  m_table->verticalHeader ()->setVisible (false);
  layout->addWidget (m_table);

  QHBoxLayout *layoutButtons = new QHBoxLayout ();

  QPushButton *btnSaveCsv = new QPushButton (tr ("Save As CSV..."));
  connect (btnSaveCsv, SIGNAL (released ()), this, SLOT (slotSaveCsv ()));
  layoutButtons->addWidget (btnSaveCsv);

  layoutButtons->addStretch ();

  QPushButton *btnClose = new QPushButton (tr ("Close"));
  connect (btnClose, SIGNAL (released ()), this, SLOT (accept ()));
  layoutButtons->addWidget (btnClose);

  layout->addLayout (layoutButtons);
}

void DlgQualityReport::populateTable (const QualityReport &report)
{
  const QList<QualityIssue> &issues = report.issues ();

  m_table->setRowCount (issues.count ());

  for (int row = 0; row < issues.count (); row++) {

    const QualityIssue &issue = issues.at (row);

    m_table->setItem (row, 0, new QTableWidgetItem (issue.curveName));
    m_table->setItem (row, 1, new QTableWidgetItem (issue.pointIdentifier));
    m_table->setItem (row, 2, new QTableWidgetItem (QualityReport::issueTypeName (issue.type)));
    m_table->setItem (row, 3, new QTableWidgetItem (QString::number (issue.posGraph.x (), 'g', 12)));
    m_table->setItem (row, 4, new QTableWidgetItem (QString::number (issue.posGraph.y (), 'g', 12)));
    m_table->setItem (row, 5, new QTableWidgetItem (issue.detail));
  }

  m_table->resizeColumnsToContents ();
}

void DlgQualityReport::slotSaveCsv ()
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgQualityReport::slotSaveCsv";

  // Local file access only, which is all a closed network deployment allows
  QString fileName = QFileDialog::getSaveFileName (this,
                                                   tr ("Save Quality Report"),
                                                   "quality_report.csv",
                                                   "CSV (*.csv)");
  if (fileName.isEmpty ()) {
    return;
  }

  if (!fileName.endsWith (".csv", Qt::CaseInsensitive)) {
    fileName += ".csv";
  }

  QFile file (fileName);
  if (!file.open (QFile::WriteOnly | QFile::Text)) {
    QMessageBox::warning (this,
                          tr ("Save Quality Report"),
                          QString ("%1 %2:\n%3.")
                          .arg (tr ("Cannot write file"))
                          .arg (fileName)
                          .arg (file.errorString ()));
    return;
  }

  QTextStream str (&file);
  str << m_reportCsv;
  file.close ();
}
