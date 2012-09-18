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
#include <QHeaderView>
#include <QIcon>
#include <QMenu>
#include <QSplitter>
#include <QTableView>
#include <QToolBar>
#include <QToolButton>

// Local includes

#include "aggregatetableview.h"
#include "pathologypropertyinfo.h"
#include "patientpropertymodel.h"
#include "patientpropertyfiltermodel.h"
#include "reporttableview.h"

class ReportWindow::ReportWindowPriv
{
public:
    ReportWindowPriv()
       : view(0),
         aggregateView(0),
         viewSplitter(0),
         toolBar(0),
         reportComboBox(0)
    {
    }

    ReportTableView      *view;
    AggregateTableView   *aggregateView;

    QSplitter  *viewSplitter;

    QToolBar   *toolBar;
    QComboBox  *reportComboBox;

    QList<QAction*> contextFilterActions;
};

ReportWindow::ReportWindow(QWidget *parent) :
    QMainWindow(parent),
    d(new ReportWindowPriv)
{
    setupView();
    setupToolbar();

    setCentralWidget(d->viewSplitter);
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
    QAction* addAction(const QString& text, int userData, QObject* receiver,
                       const char* slot = SLOT(entryActivated()), bool isCheckable = false)
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
        return action;
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
    profilesMenu->addAction(tr("Mammakarzinom"), ReportTableView::BreastCaIHCMut, this);
    profilesMenu->addAction(tr("Alle vier Tumorprofil-Entitäten"), ReportTableView::TumorprofilIHCMut, this);
    profilesButton->setMenu(profilesMenu);

    QAction* byMutationAction = d->toolBar->addAction(QIcon::fromTheme("palette"), tr("Nach Befund"));
    QToolButton* byMutationButton = static_cast<QToolButton*>(d->toolBar->widgetForAction(byMutationAction));
    byMutationButton->setPopupMode(QToolButton::InstantPopup);
    ProfileMenu* byMutationMenu = new ProfileMenu;
    byMutationMenu->addAction(tr("EGFR-Mutation"), ReportTableView::EGFRMutation, this);
    byMutationMenu->addAction(tr("PIK3CA-Mutation"), ReportTableView::PIK3Mutation, this);
    byMutationMenu->addAction(tr("PTEN-Verlust"), ReportTableView::PTENLoss, this);
    byMutationButton->setMenu(byMutationMenu);

    d->toolBar->addSeparator();

    QAction* byContextAction = d->toolBar->addAction(QIcon::fromTheme("folder"), tr("Filter nach Kontext"));
    QToolButton* byContextButton = static_cast<QToolButton*>(d->toolBar->widgetForAction(byContextAction));
    byContextButton->setPopupMode(QToolButton::InstantPopup);
    ProfileMenu* byContextMenu = new ProfileMenu;
    d->contextFilterActions <<
        byContextMenu->addAction(tr("WTZ-Tumorprofil"),  PathologyContextInfo::Tumorprofil,
                                 this, SLOT(filterByContext()), true);
    d->contextFilterActions <<
        byContextMenu->addAction(tr("BestRx"), PathologyContextInfo::BestRx,
                                 this, SLOT(filterByContext()), true);
    d->contextFilterActions <<
        byContextMenu->addAction(tr("BEZ235-Screening"), PathologyContextInfo::ScreeningBEZ235,
                                 this, SLOT(filterByContext()), true);
    d->contextFilterActions <<
        byContextMenu->addAction(tr("BGJ389-Screening"), PathologyContextInfo::ScreeningBGJ398,
                                 this, SLOT(filterByContext()), true);
    d->contextFilterActions <<
        byContextMenu->addAction(tr("BKM120-Screening"), PathologyContextInfo::ScreeningBKM120,
                                 this, SLOT(filterByContext()), true);
    d->contextFilterActions <<
        byContextMenu->addAction(tr("(kein Filter)"), PathologyContextInfo::InvalidContext,
                                 this, SLOT(filterByContext()), false);
    byContextButton->setMenu(byContextMenu);

    d->toolBar->addSeparator();

    QAction* aggregateAction = d->toolBar->addAction(QIcon::fromTheme("calculator"), tr("Zeige Auswertung"));
    aggregateAction->setCheckable(true);
    connect(aggregateAction, SIGNAL(toggled(bool)), this, SLOT(setAggregateVisible(bool)));
}

void ReportWindow::setupView()
{
    d->viewSplitter = new QSplitter(Qt::Vertical, this);
    d->view = new ReportTableView;
    d->aggregateView = new AggregateTableView;
    d->aggregateView->setSourceModel(d->view->model());
    d->viewSplitter->addWidget(d->view);
    d->viewSplitter->addWidget(d->aggregateView);

    d->aggregateView->hide();

    connect(d->aggregateView, SIGNAL(activatedReferenceIndexes(QList<QModelIndex>)),
            this, SLOT(activatedFromAggregate(QList<QModelIndex>)));

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
    PatientPropertyFilterSettings settings = d->view->filterModel()->filterSettings();
    PathologyContextInfo::Context context = (PathologyContextInfo::Context)action->data().toInt();
    if (context == PathologyContextInfo::InvalidContext)
    {
        settings.pathologyContexts.clear();
        foreach (QAction* a, d->contextFilterActions)
        {
            a->setChecked(false);
        }
    }
    else
    {
        PathologyContextInfo info(context);
        if (action->isChecked())
        {
            settings.pathologyContexts[info.id] = true;
        }
        else
        {
            settings.pathologyContexts.remove(info.id);
        }
    }
    d->view->filterModel()->setFilterSettings(settings);
}

void ReportWindow::setAggregateVisible(bool visible)
{
    d->aggregateView->setVisible(visible);
}

void ReportWindow::activatedFromAggregate(const QList<QModelIndex>& sourceReferenceIndexes)
{
    // we have a list of view->model()'s reference indexes which sum up to the contents of
    // an index which has been activated in the aggregate view.
    // We want to select full rows.
    QItemSelection selection;
    foreach (const QModelIndex& index, sourceReferenceIndexes)
    {
        selection.select(index, index);
    }
    d->view->selectionModel()->select(selection, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
}
