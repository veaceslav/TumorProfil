/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 16.05.2012
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

#include "dataaggregationmodel.h"

// Qt includes

#include <QDebug>
#include <QSet>
#include <QTimer>

// Local includes

#include "dataaggregator.h"
#include "patientpropertymodel.h"

class DataAggregationModel::DataAggregationModelPriv
{
public:

    DataAggregationModelPriv()
        : sourceModel(0),
          recomputeTimer(0)
    {
    }

    QAbstractItemModel* sourceModel;
    QList<AggregatedDatumInfo> rows;
    QList< QMap<AggregatedDatumInfo, QVariant> > columns;

    QTimer* recomputeTimer;

    PathologyPropertyInfo infoForColumn(int col)
    {
        return sourceModel->headerData(col, Qt::Horizontal,
                                       PatientPropertyModel::PathologyPropertyInfoRole)
                .value<PathologyPropertyInfo>();
    }
};

DataAggregationModel::DataAggregationModel(QObject *parent) :
    QAbstractTableModel(parent),
    d(new DataAggregationModelPriv)
{
    d->recomputeTimer = new QTimer(this);
    d->recomputeTimer->setSingleShot(true);
    d->recomputeTimer->setInterval(50);
    connect(d->recomputeTimer, SIGNAL(timeout()), this, SLOT(recompute()));
}

DataAggregationModel::~DataAggregationModel()
{
    delete d;
}

void DataAggregationModel::setSourceModel(QAbstractItemModel* sourceModel)
{
    if (d->sourceModel)
    {
        disconnect(d->sourceModel, SIGNAL(modelReset()), this, SLOT(triggerRecompute()));
        disconnect(d->sourceModel, SIGNAL(layoutChanged()), this, SLOT(triggerRecompute()));
        disconnect(d->sourceModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(triggerRecompute()));
        disconnect(d->sourceModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(triggerRecompute()));
        disconnect(d->sourceModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(triggerRecompute()));
    }

    resetData();
    d->sourceModel = sourceModel;
    computeData();

    if (d->sourceModel)
    {
        connect(d->sourceModel, SIGNAL(modelReset()), this, SLOT(triggerRecompute()));
        connect(d->sourceModel, SIGNAL(layoutChanged()), this, SLOT(triggerRecompute()));
        connect(d->sourceModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(triggerRecompute()));
        connect(d->sourceModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(triggerRecompute()));
        connect(d->sourceModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(triggerRecompute()));
    }
}

QAbstractItemModel* DataAggregationModel::sourceModel() const
{
    return d->sourceModel;
}

void DataAggregationModel::triggerRecompute()
{
    if (!d->recomputeTimer->isActive())
    {
        d->recomputeTimer->start();
    }
}

void DataAggregationModel::recompute()
{
    d->recomputeTimer->stop();
    resetData();
    computeData();
}

void DataAggregationModel::resetData()
{
    if (!d->rows.isEmpty())
    {
        beginRemoveRows(QModelIndex(), 0, d->rows.size() - 1);
        d->rows.clear();
        endRemoveRows();
    }
    if (!d->columns.isEmpty())
    {
        beginRemoveColumns(QModelIndex(), 0, d->columns.size() - 1);
        d->columns.clear();
        endRemoveColumns();
    }
}

void DataAggregationModel::computeData()
{
    QSet<AggregatedDatumInfo> rowFields;
    QList< QMap<AggregatedDatumInfo, QVariant> > cols;
    QList<AggregatedDatumInfo> rows;

    const int columns = d->sourceModel->columnCount();
    for (int col=0; col<columns; col++)
    {
        QMap<AggregatedDatumInfo,QVariant> map;
        // Is the column displaying a Property defined by a PathologyPropertyInfo?
        PathologyPropertyInfo info = d->infoForColumn(col);

        if (info.isValid())
        {
            // Aggregate property results
            DataAggregator aggregator(info);
            const int sourceRows = d->sourceModel->rowCount();
            for (int row=0; row<sourceRows; row++)
            {
                QModelIndex index = d->sourceModel->index(row, col);
                Property prop = index.data(PatientPropertyModel::PathologyPropertyRole).value<Property>();
                aggregator << prop;
            }
            map = aggregator.values();
            for (QMap<AggregatedDatumInfo,QVariant>::const_iterator it=map.begin(); it != map.end(); ++it)
            {
                rowFields << it.key();
            }
        }
        cols << map;
    }
    rows = rowFields.toList();
    qSort(rows);

    layoutAboutToBeChanged();
    d->columns = cols;
    d->rows = rows;
    layoutChanged();
}

QVariant DataAggregationModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= d->rows.size() || index.column() >= d->columns.size())
    {
        return QVariant();
    }

    const AggregatedDatumInfo& info = d->rows.at(index.row());
    if (!info.isValid())
    {
        return QVariant();
    }
    const QMap<AggregatedDatumInfo, QVariant>& dataMap = d->columns.at(index.column());
    switch (role)
    {
    case Qt::DisplayRole:
    {
        QMap<AggregatedDatumInfo, QVariant>::const_iterator it = dataMap.find(info);
        if (it != dataMap.constEnd())
        {
            switch (info.valueType)
            {
            case AggregatedDatumInfo::InvalidValue:
                break;
            case AggregatedDatumInfo::AbsoluteValue:
                return it.value();
            case AggregatedDatumInfo::PercentageValue:
            case AggregatedDatumInfo::ConfidenceUpper:
            case AggregatedDatumInfo::ConfidenceLower:
                return it.value();//formatAsPercentage(it.value());
            }
        }
        return QVariant();
    }
    case PathologyPropertyInfoRole:
        return QVariant::fromValue(d->infoForColumn(index.column()));
    case AggregatedDatumInfoRole:
        return QVariant::fromValue(info);
    }
    return QVariant();
}

QVariant DataAggregationModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (!d->sourceModel || role != Qt::DisplayRole)
    {
        return QVariant();
    }

    if (orientation == Qt::Horizontal)
    {
        return d->sourceModel->headerData(section, orientation, role);
    }
    else
    {
        const AggregatedDatumInfo& info = d->rows.at(section);
        if (info.isValid())
        {
            return info.label();
        }
    }
    return QVariant();
}

QModelIndex DataAggregationModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid() || !d->sourceModel ||
            row > d->rows.size() || column > d->columns.size())
    {
        return QModelIndex();
    }
    return createIndex(row, column, 0);
}

int DataAggregationModel::columnCount(const QModelIndex& parent) const
{
    if (!d->sourceModel)
    {
        return 0;
    }
    return d->sourceModel->columnCount();
}

int DataAggregationModel::rowCount(const QModelIndex& parent ) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return d->rows.size();
}

Qt::ItemFlags DataAggregationModel::flags(const QModelIndex& index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

bool DataAggregationModel::hasChildren(const QModelIndex& parent ) const
{
    if (parent.isValid())
    {
        return false;
    }
    return true;
}

QModelIndex DataAggregationModel::parent(const QModelIndex& index) const
{
    return QModelIndex();
}

QList<QModelIndex> DataAggregationModel::sourceModelReferenceIndexes(const QModelIndex& index)
{
    QList<QModelIndex> results;

    PathologyPropertyInfo info = index.data(DataAggregationModel::PathologyPropertyInfoRole)
            .value<PathologyPropertyInfo>();
    AggregatedDatumInfo datumInfo = index.data(DataAggregationModel::AggregatedDatumInfoRole)
            .value<AggregatedDatumInfo>();

    if (!info.isValid() || !datumInfo.isValid())
    {
        return results;
    }

    DataAggregator aggregator(info);
    const int sourceRows = d->sourceModel->rowCount();
    const int col = index.column(); // our columns and source model columns are identical
    for (int row=0; row<sourceRows; row++)
    {
        QModelIndex index = d->sourceModel->index(row, col);
        Property prop = index.data(PatientPropertyModel::PathologyPropertyRole).value<Property>();
        if (aggregator.isCountedAs(prop, datumInfo))
        {
            results << index;
        }
    }
    return results;
}
