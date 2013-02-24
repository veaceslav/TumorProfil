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

#include "actionableresultchecker.h"
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
    QStringList extraColumnTitles;
    QList< QList<PathologyPropertyInfo> > extraCombinations; // corresponding to extraColumnTitles

    QTimer* recomputeTimer;

    DataAggregation::FieldNature natureOfColumn(int col)
    {
        return  (DataAggregation::FieldNature)
                sourceModel->headerData(col, Qt::Horizontal,
                                       PatientPropertyModel::DataAggregationNatureRole).toInt();
    }

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

static bool operator<(const QList<PathologyPropertyInfo>& a,
                      const QList<PathologyPropertyInfo>& b)
{
    if (a.size() == b.size())
    {
        const int size = a.size();
        for (int i=0; i<size; i++)
        {
            if (a[i] == b[i])
            {
                continue;
            }
            return a[i] < b[i];
        }
    }
    return a.size() < b.size();
}

void DataAggregationModel::computeData()
{
    QSet<AggregatedDatumInfo> rowFields;
    QList< QMap<AggregatedDatumInfo, QVariant> > cols;
    QList<AggregatedDatumInfo> rows;

    // 1) Aggregation of the source model's columns

    const int columns = d->sourceModel->columnCount();
    for (int col=0; col<columns; col++)
    {
        QMap<AggregatedDatumInfo,QVariant> map;

        DataAggregation::FieldNature nature = d->natureOfColumn(col);
        if (nature == DataAggregation::PathologyResult)
        {
            // Is the column displaying a Property defined by a PathologyPropertyInfo?
            PathologyPropertyInfo info = d->infoForColumn(col);

            if (info.isValid())
            {
                qDebug() << "Column" << col << info.id ;
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
            }
            else
            {
                qDebug() << "Error: Field says it has PathologyResult nature, but gives no field info.";
            }
        }
        else
        {
            DataAggregator aggregator(nature);
            const int sourceRows = d->sourceModel->rowCount();
            for (int row=0; row<sourceRows; row++)
            {
                QModelIndex index = d->sourceModel->index(row, col);
                aggregator << index.data(PatientPropertyModel::VariantDataRole);
            }
            map = aggregator.values();
        }

        for (QMap<AggregatedDatumInfo,QVariant>::const_iterator it=map.begin(); it != map.end(); ++it)
        {
            rowFields << it.key();
        }
        cols << map;
    }

    // 2) Combinations of actionable results (single-only numbers and double mutants)

    QStringList extraColumnTitles;
    QMap< QList<PathologyPropertyInfo>, DataAggregator* > actionableCombinations;
    const int rowCount = d->sourceModel->rowCount();
    // Find out which combinations of actionable results exist
    for (int row=0; row<rowCount; ++row)
    {
        QModelIndex index = d->sourceModel->index(row, 0);
        Patient::Ptr p = PatientModel::retrievePatient(index);

        ActionableResultChecker checker(p, ActionableResultChecker::IncludeKRAS);
        QList<PathologyPropertyInfo> combination = checker.actionableResults();
        if (!actionableCombinations.contains(combination))
        {
            actionableCombinations.insert(combination, new DataAggregator(DataAggregation::Boolean));
        }
    }
    // Aggregate info
    QMap< QList<PathologyPropertyInfo>,  DataAggregator* >::const_iterator it;
    for (int row=0; row<rowCount; ++row)
    {
        QModelIndex index = d->sourceModel->index(row, 0);
        Patient::Ptr p = PatientModel::retrievePatient(index);

        ActionableResultChecker checker(p, ActionableResultChecker::IncludeKRAS);
        // Extra measure: If a patient has a combination of two results, he may fit into three combinations etc.
        // Give the patient to the last in the list (which has the largest number of properties, see operator< above)
        QList<DataAggregator*> positiveAggregators;
        for (it = actionableCombinations.begin(); it != actionableCombinations.end(); ++it)
        {
            QVariant value = checker.hasResults(it.key());
            DataAggregator* aggregator = it.value();
            if (value.toBool())
            {
                positiveAggregators << aggregator;
            }
            else
            {
                *aggregator << value;
            }
        }
        if (!positiveAggregators.isEmpty())
        {
            *positiveAggregators.takeLast() << true;
            foreach (DataAggregator* aggregator, positiveAggregators)
            {
                // exclusive for double mutants
                *aggregator << false;
                // inclusive for double mutants
                //*aggregator << true;
            }
        }
    }
    // Read information from DataAggregators, add extra columns
    for (it = actionableCombinations.begin(); it != actionableCombinations.end(); ++it)
    {
        QMap<AggregatedDatumInfo,QVariant> map = it.value()->values();
        for (QMap<AggregatedDatumInfo,QVariant>::const_iterator it2=map.begin(); it2 != map.end(); ++it2)
        {
            rowFields << it2.key();
        }
        cols << map;
        QStringList titles;
        foreach (const PathologyPropertyInfo& info, it.key())
        {
            titles << info.plainTextLabel();
        }
        if (titles.isEmpty())
        {
            titles << "Kein relevanter Befund";
        }
        extraColumnTitles << titles.join(", ");

    }
    qDeleteAll(actionableCombinations);

    // Apply changes to model

    // Bring rows to a sorted list
    rows = rowFields.toList();
    qSort(rows);

    // Change data
    layoutAboutToBeChanged();
    d->columns = cols;
    d->rows = rows;
    d->extraColumnTitles = extraColumnTitles;
    d->extraCombinations = actionableCombinations.keys();
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
        if (section < d->sourceModel->columnCount())
        {
            return d->sourceModel->headerData(section, orientation, role);
        }
        int extraColumn = section - d->sourceModel->columnCount();
        if (extraColumn < d->extraColumnTitles.size())
        {
            return d->extraColumnTitles.at(extraColumn);
        }
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
    return d->sourceModel->columnCount() + d->extraColumnTitles.size();
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

    // 1) Simple aggregation over source model
    if (index.column() <= d->sourceModel->columnCount())
    {

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
    }
    // 2) Actionable results-based aggregation
    else
    {
        const int extraColumn = index.column() - d->sourceModel->columnCount();
        const int rowCount = d->sourceModel->rowCount();
        // Iterate individual patients and check if combination applies.
        // Combination of column is stored in extraCombinations.
        for (int row=0; row<rowCount; ++row)
        {
            QModelIndex index = d->sourceModel->index(row, 0);
            Patient::Ptr p = PatientModel::retrievePatient(index);

            ActionableResultChecker checker(p, ActionableResultChecker::IncludeKRAS);
            if (checker.hasResults(d->extraCombinations[extraColumn]).toBool())
            {
                results << index;
            }
        }
    }
    return results;
}
