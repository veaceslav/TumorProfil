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
#include <QDateEdit>
#include <QDebug>
#include <QHeaderView>
#include <QIcon>
#include <QMenu>
#include <QSplitter>
#include <QTableView>
#include <QToolBar>
#include <QToolButton>
#include <QWidgetAction>

// Local includes

#include "aggregatetableview.h"
#include "pathologypropertyinfo.h"
#include "patientpropertymodel.h"
#include "patientpropertyfiltermodel.h"
#include "patientpropertymodelviewadapter.h"
#include "reporttableview.h"

class ReportWindow::ReportWindowPriv
{
public:
    ReportWindowPriv()
       : view(0),
         aggregateView(0),
         viewSplitter(0)
    {
    }

    ReportTableView      *view;
    AggregateTableView   *aggregateView;

    QSplitter  *viewSplitter;
};

ReportWindow::ReportWindow(QWidget *parent) :
    FilterMainWindow(parent),
    d(new ReportWindowPriv)
{
    setupView();
    setupToolbar();

    setCentralWidget(d->viewSplitter);
    setWindowTitle(tr("Analyse"));
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
}

ReportWindow::~ReportWindow()
{
    delete d;
}

ReportTableView* ReportWindow::view() const
{
    return d->view;
}

void ReportWindow::setupToolbar()
{
    FilterMainWindow::setupToolbar();

    toolBar()->addSeparator();

    QAction* aggregateAction = toolBar()->addAction(QIcon::fromTheme("calculator"), tr("Zeige Auswertung"));
    aggregateAction->setCheckable(true);
    connect(aggregateAction, SIGNAL(toggled(bool)), this, SLOT(setAggregateVisible(bool)));

    QAction* clearFilterAction = toolBar()->addAction(QIcon::fromTheme("link_break"), tr("Zweitfilter zurücksetzen"));
    connect(clearFilterAction, SIGNAL(triggered()), adapter(), SLOT(clearFilter()));
}

void ReportWindow::setupView()
{
    d->viewSplitter = new QSplitter(Qt::Vertical, this);
    d->view = new ReportTableView;
    d->view->setAdapter(adapter());
    d->aggregateView = new AggregateTableView;
    d->aggregateView->setSourceModel(d->view->model());
    d->viewSplitter->addWidget(d->view);
    d->viewSplitter->addWidget(d->aggregateView);

    d->aggregateView->hide();

    connect(d->aggregateView, SIGNAL(activatedReferenceIndexes(QList<QModelIndex>)),
            this, SLOT(activatedFromAggregate(QList<QModelIndex>)));

    connect(d->view, SIGNAL(filterAdded(PathologyPropertyInfo,QVariant)),
            adapter(), SLOT(addFilter(PathologyPropertyInfo,QVariant)));
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
