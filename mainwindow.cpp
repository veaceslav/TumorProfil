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

#include <QAction>
#include <QDebug>
#include <QMessageBox>
#include <QSplitter>
#include <QStackedLayout>
#include <QToolBar>

// Local includes

#include "diseasetabwidget.h"
#include "patientdisplay.h"
#include "patiententerform.h"
#include "patientlistview.h"
#include "patientmanager.h"
#include "pathologywidgetgenerator.h"
#include "tnmwidget.h"

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

    QToolBar         *toolBar;

    Patient::Ptr      currentPatient;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      d(new MainWindowPriv)
{
    setupUI();
    setupToolbar();
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
                          tr("�nderungen verwerfen"),
                          this, SLOT(discardChanges()));
}

void MainWindow::setupUI()
{
    d->splitter = new QSplitter;

    d->listView = new PatientListView;

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

    d->splitter->addWidget(d->listView);
    d->splitter->addWidget(rightWidget);

    setCentralWidget(d->splitter);

    connect(d->patientEnterForm, SIGNAL(editingFinished(Patient)),
            this, SLOT(newPatientEntered(Patient)));

    connect(d->listView, SIGNAL(activated(Patient::Ptr)),
            this, SLOT(setPatient(Patient::Ptr)));

    connect(d->tabWidget, SIGNAL(editingFinished()),
            this, SLOT(enterNewPatient()));

    resize(800, 600);
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
    if (QMessageBox::warning(this, tr("�nderungen verwerfen"),
                              tr("M�chten Sie allen �nderungen zu diesem Patienten verwerfen?"),
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
