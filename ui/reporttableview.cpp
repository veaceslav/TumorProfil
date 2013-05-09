/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 23.02.2012
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

#include "reporttableview.h"

// Qt includes

#include <QDebug>
#include <QMenu>
#include <QHeaderView>

// Local includes

#include "patientpropertymodel.h"
#include "patientpropertyfiltermodel.h"
#include "patientpropertymodelviewadapter.h"
#include "pathologypropertyinfo.h"

class ReportTableView::ReportTableViewPriv
{
public:
    ReportTableViewPriv()
        : adapter(0)
    {
    }

    PatientPropertyModelViewAdapter* adapter;
};

ReportTableView::ReportTableView(QWidget *parent) :
    AnalysisTableView(parent),
    d(new ReportTableViewPriv)
{
    verticalHeader()->hide();

    sortByColumn(0, Qt::AscendingOrder);
    setSortingEnabled(true);

    connect(this, SIGNAL(activated(QModelIndex)),
            this, SLOT(indexActivated(QModelIndex)));
}

void ReportTableView::setAdapter(PatientPropertyModelViewAdapter* adapter)
{
    d->adapter = adapter;
    setModel(d->adapter->filterModel());
}

ReportTableView::~ReportTableView()
{
    delete d;
}

void ReportTableView::indexActivated(const QModelIndex& index)
{
    Patient::Ptr p = PatientModel::retrievePatient(index);
    if (p)
    {
        emit activated(p);
    }
}

void ReportTableView::addContextMenuActions(QMenu *menu)
{
    AnalysisTableView::addContextMenuActions(menu);
    menu->addAction(tr("Hiernach filtern"), this, SLOT(filterWithCurrentSelection()));
}

void ReportTableView::filterWithCurrentSelection()
{
    qDebug() << currentIndex();
    if (!currentIndex().isValid())
    {
        return;
    }

    QVariant infoVariant = currentIndex().model()->headerData(currentIndex().column(), Qt::Horizontal,
                                                              PatientPropertyModel::PathologyPropertyInfoRole);
    qDebug() << infoVariant;

    if (infoVariant.isNull())
    {
        return;
    }

    PathologyPropertyInfo info = infoVariant.value<PathologyPropertyInfo>();
    qDebug() << info.id;
    // Filter by exact IHC score value
    //emit filterAdded(info, currentIndex().data(PatientPropertyModel::VariantDataRole));
    // Simplify IHC scores to boolean
    emit filterAdded(info, currentIndex().data(PatientPropertyModel::VariantDataRole).toBool());
}
