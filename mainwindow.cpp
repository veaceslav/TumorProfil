/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 2012-01-22
 *
 * Copyright (C) 2012 by Marcel Wiesweg <marcel dot wiesweg at uk-essen dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "mainwindow.h"

// Qt includes

#include <QAbstractItemModel>
#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QDebug>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QStackedLayout>
#include <QStatusBar>
#include <QTextEdit>
#include <QToolBar>

// Local includes

#include "diseasetabwidget.h"
#include "patientdisplay.h"
#include "patiententerform.h"
#include "patientlistview.h"
#include "patientmanager.h"
#include "patientmodel.h"
#include "pathologywidgetgenerator.h"
#include "reporttableview.h"
#include "reportwindow.h"
#include "tnmwidget.h"

class ModelFilterLineEdit : public QLineEdit
{
public:
    ModelFilterLineEdit(QSortFilterProxyModel* model)
        : filterModel(model)
    {
        connect(this, SIGNAL(textChanged(QString)),
                model, SLOT(setFilterFixedString(QString)));
        setPlaceholderText(tr("Suche Patient"));
    }

    void keyPressEvent(QKeyEvent *e)
    {
        if (e->key() == Qt::Key_Escape)
        {
            clear();
            return;
        }
        QLineEdit::keyPressEvent(e);
    }

protected:

    QSortFilterProxyModel* filterModel;
};

class MainWindow::MainWindowPriv
{
public:
    MainWindowPriv()
    {
    }

    QSplitter        *splitter;
    PatientListView  *listView;
    PatientDisplay   *patientDisplay;
    DiseaseTabWidget *tabWidget;
    PatientEnterForm *patientEnterForm;
    QVBoxLayout      *displayWorkLayout;
    QStackedLayout   *workLayout;
    ModelFilterLineEdit *searchBar;
    QCheckBox        *tumorprofilCheckbox;

    QToolBar         *toolBar;
    QStatusBar       *statusBar;

    Patient::Ptr      currentPatient;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      d(new MainWindowPriv)
{
    setupUI();
    setupToolbar();
    setupStatusBar();
    setFocusPolicy(Qt::ClickFocus);
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    if (e->matches(QKeySequence::Paste) )
    {
        /*QTextEdit *e = new QTextEdit;
        e->setHtml(QApplication::clipboard()->mimeData()->html());
        e->show();
        */
        foreach (const QString& format, QApplication::clipboard()->mimeData()->formats())
        {
            qDebug() << format;
            qDebug() << QApplication::clipboard()->mimeData()->data(format).left(1000);
        }
    }
}

MainWindow::~MainWindow()
{
    delete d;
}

void MainWindow::setupToolbar()
{
    d->toolBar = addToolBar(tr("Aktionen"));
    d->toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    d->toolBar->addAction(QIcon::fromTheme("add"),
                          tr("Neuer Patient"),
                          this, SLOT(enterNewPatient()));

    d->toolBar->addAction(QIcon::fromTheme("cancel"),
                          tr("Änderungen verwerfen"),
                          this, SLOT(discardChanges()));

    d->toolBar->addSeparator();

    d->toolBar->addAction(QIcon::fromTheme("report"),
                          tr("Analyse"),
                          this, SLOT(showReport()));
}

void MainWindow::setupStatusBar()
{
    statusBar();

    connect(d->listView->model(), SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(patientNumberChanged()));
    connect(d->listView->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(patientNumberChanged()));
    connect(d->listView->model(), SIGNAL(modelReset()),
            this, SLOT(patientNumberChanged()));

    patientNumberChanged();
}

void MainWindow::setupUI()
{
    d->splitter = new QSplitter;

    QWidget* leftSidebarWidget = new QWidget;
    QVBoxLayout* patientListLayout = new QVBoxLayout;
    d->listView = new PatientListView;
    d->searchBar = new ModelFilterLineEdit(d->listView->filterModel());
    d->tumorprofilCheckbox = new QCheckBox(QObject::tr("nur Tumorprofil"));
    patientListLayout->addWidget(d->listView);
    patientListLayout->addWidget(d->searchBar);
    patientListLayout->addWidget(d->tumorprofilCheckbox);
    leftSidebarWidget->setLayout(patientListLayout);

    QWidget* rightWidget = new QWidget;
    d->displayWorkLayout = new QVBoxLayout;

    d->patientDisplay    = new PatientDisplay;

    d->workLayout        = new QStackedLayout;
    d->patientEnterForm  = new PatientEnterForm;
    d->tabWidget         = new DiseaseTabWidget;
    d->workLayout->addWidget(d->patientEnterForm);
    d->workLayout->addWidget(d->tabWidget);
    d->workLayout->setCurrentWidget(d->patientEnterForm);

    d->displayWorkLayout->addWidget(d->patientDisplay);
    d->displayWorkLayout->addLayout(d->workLayout);
    d->displayWorkLayout->addStretch();
    rightWidget->setLayout(d->displayWorkLayout);

    d->splitter->addWidget(leftSidebarWidget);
    d->splitter->addWidget(rightWidget);

    setCentralWidget(d->splitter);

    connect(d->patientEnterForm, SIGNAL(editingFinished(Patient)),
            this, SLOT(newPatientEntered(Patient)));

    connect(d->patientEnterForm, SIGNAL(nameEdited(Patient)),
            this, SLOT(patientNameEdited(Patient)));

    connect(d->listView, SIGNAL(activated(Patient::Ptr)),
            this, SLOT(setPatient(Patient::Ptr)));

    connect(d->tabWidget, SIGNAL(editingFinished()),
            this, SLOT(enterNewPatient()));

    connect(d->searchBar, SIGNAL(returnPressed()),
            this, SLOT(selectFilteredPatient()));

    connect(d->tumorprofilCheckbox, SIGNAL(toggled(bool)),
            d->listView, SLOT(setFilterByTumorprofil(bool)));

    resize(1000, 800);
    d->splitter->setSizes(QList<int>() << 250 << 550);
}

void MainWindow::enterNewPatient()
{
    setPatient(Patient::Ptr());
}

void MainWindow::setPatient(const Patient::Ptr& p)
{
    if (p && p == d->currentPatient)
    {
        return;
    }

    save();
    d->currentPatient = p;

    if (d->currentPatient)
    {
        d->patientDisplay->setPatient(p);
        d->workLayout->setCurrentWidget(d->tabWidget);
        d->tabWidget->setPatient(p);
        d->listView->setCurrentPatient(p);
    }
    else
    {
        d->patientDisplay->clear();
        d->patientEnterForm->newPatient();
        d->workLayout->setCurrentWidget(d->patientEnterForm);
    }
}

void MainWindow::save()
{
    if (!d->currentPatient)
    {
        return;
    }
    d->tabWidget->save(d->currentPatient);
    PatientManager::instance()->updateData(d->currentPatient);
}

void MainWindow::newPatientEntered(const Patient& p)
{
    Patient::Ptr ptr = PatientManager::instance()->addPatient(p);
    setPatient(ptr);
}

void MainWindow::discardChanges()
{
    if (QMessageBox::warning(this, tr("Änderungen verwerfen"),
                              tr("Möchten Sie allen Änderungen zu diesem Patienten verwerfen?"),
                              QMessageBox::Discard | QMessageBox::No, QMessageBox::Discard)
            == QMessageBox::Discard)
    {
        d->currentPatient = Patient::Ptr();
        setPatient(d->currentPatient);
    }
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    save();
    QMainWindow::closeEvent(e);
}

void MainWindow::showReport()
{
    ReportWindow* report = new ReportWindow;
    report->setAttribute(Qt::WA_DeleteOnClose);
    report->setAttribute(Qt::WA_QuitOnClose, false);

    connect(report->view(), SIGNAL(activated(Patient::Ptr)),
            this, SLOT(setPatient(Patient::Ptr)));
    connect(report->view(), SIGNAL(activated(Patient::Ptr)),
            this, SLOT(raise()));

    report->showMaximized();
}

void MainWindow::patientNameEdited(const Patient &p)
{
    // TODO: improve auto-completion
    QList<Patient::Ptr> candidates = PatientManager::instance()->findPatients(p.surname, p.firstName);
    if (!candidates.isEmpty())
    {
        d->patientEnterForm->setValues(*candidates.first());
    }
}

void MainWindow::patientNumberChanged()
{
    const int rows = d->listView->model()->rowCount();
    int profilCount = 0;
    for (int r=0; r<rows; r++)
    {
        if (d->listView->model()->index(r, 0).data(PatientModel::HasTumorprofilRole).toBool())
        {
            profilCount++;
        }
    }
    statusBar()->showMessage(tr("%1 Patienten, davon %2 mit einem Tumorprofil").arg(rows).arg(profilCount));
}

void MainWindow::selectFilteredPatient()
{
    if (d->listView->currentIndex().isValid())
    {
        setPatient(d->listView->currentPatient());
    }
    else if (d->listView->model()->rowCount())
    {
        QModelIndex index = d->listView->model()->index(0,0);
        d->listView->setCurrentIndex(index);
        setPatient(d->listView->currentPatient());
    }
    d->searchBar->clear();
}
