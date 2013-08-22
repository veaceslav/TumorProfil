/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 23.03.2013
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

#include "historypatientlistview.h"

// Local includes

#include "patientpropertymodel.h"
#include "patientpropertyfiltermodel.h"
#include "patientpropertymodelviewadapter.h"

class HistoryPatientListView::HistoryPatientListViewPriv
{
public:
    HistoryPatientListViewPriv()
        : adapter(0)
    {
    }

    PatientPropertyModelViewAdapter* adapter;
};

HistoryPatientListView::HistoryPatientListView(QWidget *parent) :
    QTreeView(parent),
    d(new HistoryPatientListViewPriv)
{
    setSortingEnabled(true);

    connect(this, SIGNAL(activated(QModelIndex)),
            this, SLOT(indexActivated(QModelIndex)));
}

void HistoryPatientListView::setAdapter(PatientPropertyModelViewAdapter* adapter)
{
    d->adapter = adapter;
    setModel(d->adapter->filterModel());
    sortByColumn(0, Qt::AscendingOrder);
    // hide all except first 3 columns
    for (int i=3; i<model()->columnCount();++i)
    {
        setColumnHidden(i, true);
    }
}

void HistoryPatientListView::indexActivated(const QModelIndex& index)
{
    Patient::Ptr p = PatientModel::retrievePatient(index);
    if (p)
    {
        emit activated(p);
    }
}

