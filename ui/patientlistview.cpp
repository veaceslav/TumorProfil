/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 30.01.2012
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

#include "patientlistview.h"

// Qt includes

#include <QSortFilterProxyModel>

// Local includes

#include "patient.h"
#include "patientmodel.h"

class PatientListView::PatientListViewPriv
{
public:
    PatientListViewPriv()
    {
    }

    PatientModel          *model;
    QSortFilterProxyModel *sortFilterModel;
};

PatientListView::PatientListView(QWidget *parent) :
    QTreeView(parent),
    d(new PatientListViewPriv)
{
    d->model = new PatientModel(this);
    d->sortFilterModel = new QSortFilterProxyModel(this);

    d->sortFilterModel->setSourceModel(d->model);
    setModel(d->model);

    d->sortFilterModel->sort(0);

    connect(this, SIGNAL(activated(QModelIndex)),
            this, SLOT(slotActivated(QModelIndex)));
}

void PatientListView::setCurrentPatient(const Patient::Ptr& p)
{
    setCurrentIndex(d->model->indexForPatient(p));
}

void PatientListView::slotActivated(const QModelIndex &index)
{
    emit activated(d->model->patientForIndex(index));
}
