/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 22.02.2012
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

#include "reportwindow.h"

// Qt includes

#include <QAction>
#include <QComboBox>
#include <QDebug>
#include <QIcon>
#include <QMenu>
#include <QTableView>
#include <QToolBar>
#include <QToolButton>

// Local includes

#include "pathologypropertyinfo.h"
#include "patientpropertyfiltermodel.h"
#include "reporttableview.h"

class ReportWindow::ReportWindowPriv
{
public:
    ReportWindowPriv()
       :  view(0),
          toolBar(0),
          reportComboBox(0)
    {
    }

    ReportTableView      *view;

    QToolBar   *toolBar;
    QComboBox  *reportComboBox;
};

ReportWindow::ReportWindow(QWidget *parent) :
    QMainWindow(parent),
    d(new ReportWindowPriv)
{
    setupView();
    setupToolbar();

    setCentralWidget(d->view);
    setWindowTitle(tr("Analyse"));
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    d->view->setReportType(ReportTableView::OverviewReport);
}

ReportWindow::~ReportWindow()
{
    delete d;
}

ReportTableView* ReportWindow::view() const
{
    return d->view;
}

class ProfileMenu : public QMenu
{
public:
    void addAction(const QString& text, int userData, QObject* receiver, const char* slot = SLOT(entryActivated()),
                   bool isCheckable = false)
    {
        QAction* action = QMenu::addAction(text);
        action->setData(userData);
        if (isCheckable)
        {
            action->setCheckable(true);
            connect(action, SIGNAL(toggled(bool)), receiver, slot);
        }
        else
        {
            connect(action, SIGNAL(triggered()), receiver, slot);
        }
    }
};

void ReportWindow::setupToolbar()
{
    d->toolBar = addToolBar(tr("Auswahl"));

    QAction* overviewAction = d->toolBar->addAction(QIcon::fromTheme("table"), tr("Übersicht"),
                                                    this, SLOT(entryActivated()));
    overviewAction->setData((int)ReportTableView::OverviewReport);

    QAction* profilesAction = d->toolBar->addAction(QIcon::fromTheme("photo"), tr("Nach Entität"));
    QToolButton* profilesButton = static_cast<QToolButton*>(d->toolBar->widgetForAction(profilesAction));
    profilesButton->setPopupMode(QToolButton::InstantPopup);
    ProfileMenu* profilesMenu = new ProfileMenu;
    profilesMenu->addAction(tr("Adenokarzinom Lunge"), ReportTableView::PulmonaryAdenoIHCMut, this);
    profilesMenu->addAction(tr("Plattenepithelkarzinom Lunge"), ReportTableView::PulmonarySquamousIHCMut, this);
    profilesMenu->addAction(tr("Kolorektales Karzinom"), ReportTableView::CRCIHCMut, this);
    profilesButton->setMenu(profilesMenu);

    QAction* byMutationAction = d->toolBar->addAction(QIcon::fromTheme("palette"), tr("Nach Befund"));
    QToolButton* byMutationButton = static_cast<QToolButton*>(d->toolBar->widgetForAction(byMutationAction));
    byMutationButton->setPopupMode(QToolButton::InstantPopup);
    ProfileMenu* byMutationMenu = new ProfileMenu;
    byMutationMenu->addAction(tr("EGFR-Mutation"), ReportTableView::EGFRMutation, this);
    byMutationMenu->addAction(tr("PIK3CA-Mutation"), ReportTableView::PIK3Mutation, this);
    byMutationMenu->addAction(tr("PTEN-Verlust"), ReportTableView::PTENLoss, this);
    byMutationButton->setMenu(byMutationMenu);

    QAction* byContextAction = d->toolBar->addAction(QIcon::fromTheme("folder"), tr("Filter nach Kontext"));
    QToolButton* byContextButton = static_cast<QToolButton*>(d->toolBar->widgetForAction(byContextAction));
    byContextButton->setPopupMode(QToolButton::InstantPopup);
    ProfileMenu* byContextMenu = new ProfileMenu;
    byContextMenu->addAction(tr("WTZ-Tumorprofil"),  PathologyContextInfo::Tumorprofil, this, SLOT(filterByContext()), true);
    byContextMenu->addAction(tr("BEZ235-Screening"), PathologyContextInfo::ScreeningBEZ235, this, SLOT(filterByContext()), true);
    byContextMenu->addAction(tr("BGJ389-Screening"), PathologyContextInfo::ScreeningBGJ398, this, SLOT(filterByContext()), true);
    byContextButton->setMenu(byContextMenu);

    /*d->reportComboBox = new QComboBox;
    d->toolBar->addWidget(d->reportComboBox);

    d->reportComboBox->setInsertPolicy(QComboBox::NoInsert);
    d->reportComboBox->setEditable(false);
    d->reportComboBox->addItem("Übersicht", (int)ReportTableView::OverviewReport);
    d->reportComboBox->addItem(tr("Adeno Lunge"), (int)ReportTableView::PulmonaryAdenoIHCMut);
    d->reportComboBox->addItem(tr("Plattenepithel Lunge"), (int)ReportTableView::PulmonarySquamousIHCMut);
    d->reportComboBox->addItem(tr("Kolorektal"), (int)ReportTableView::CRCIHCMut);

    connect(d->reportComboBox, SIGNAL(activated(int)),
            this, SLOT(entryActivated(int)));*/
}

void ReportWindow::setupView()
{
    d->view = new ReportTableView;

    /*connect(this, SIGNAL(activated(QModelIndex)),
            this, SLOT(slotActivated(QModelIndex)));*/
}

void ReportWindow::entryActivated()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action)
    {
        return;
    }
    d->view->setReportType(action->data().toInt());
}

void ReportWindow::filterByContext()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action)
    {
        return;
    }
    PathologyContextInfo info((PathologyContextInfo::Context)action->data().toInt());
    PatientPropertyFilterSettings settings = d->view->filterModel()->filterSettings();
    if (action->isChecked())
    {
        settings.pathologyContexts[info.id] = true;
    }
    else
    {
        settings.pathologyContexts.remove(info.id);
    }
    d->view->filterModel()->setFilterSettings(settings);
}

