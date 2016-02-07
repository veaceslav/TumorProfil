/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 20.05.2012
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

#include "aggregatetableview.h"

// Qt includes

#include <QHeaderView>

// Local includes

#include "dataaggregationmodel.h"
#include "dataaggregationfiltermodel.h"
#include "patientpropertymodel.h"

class AggregateTableView::AggregateTableViewPriv
{
public:
    AggregateTableViewPriv()
        : model(0)
    {
    }

    DataAggregationModel* model;
    DataAggregationFilterModel* filterModel;
};

AggregateTableView::AggregateTableView(QWidget *parent) :
    AnalysisTableView(parent),
    d(new AggregateTableViewPriv)
{
    installDelegateToDisplayPercentages();
    d->model = new DataAggregationModel(this);
    d->filterModel = new DataAggregationFilterModel(this);
    d->filterModel->setSourceModel(d->model);
    setModel(d->filterModel);

    connect(this, SIGNAL(activated(QModelIndex)),
            this, SLOT(slotActivated(QModelIndex)));
}

AggregateTableView::~AggregateTableView()
{
    delete d;
}

void AggregateTableView::setSourceModel(QAbstractItemModel* source)
{
    d->model->setSourceModel(source);
}

DataAggregationModel* AggregateTableView::dataAggregationModel() const
{
    return d->model;
}

void  AggregateTableView::slotActivated(const QModelIndex& index)
{
    QList<QModelIndex> referenceIndexes = d->model->sourceModelReferenceIndexes(d->filterModel->mapToSource(index));
    emit activatedReferenceIndexes(referenceIndexes);
}
