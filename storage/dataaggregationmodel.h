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

#ifndef DATAAGGREGATIONMODEL_H
#define DATAAGGREGATIONMODEL_H

#include <QAbstractTableModel>

class DataAggregationModel : public QAbstractTableModel
{
    Q_OBJECT
public:

    enum Roles
    {
        PathologyPropertyInfoRole = Qt::UserRole,
        AggregatedDatumInfoRole   = Qt::UserRole + 1
    };

    explicit DataAggregationModel(QObject *parent = 0);
    ~DataAggregationModel();

    void setSourceModel(QAbstractItemModel* sourceModel);
    QAbstractItemModel* sourceModel() const;
    
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex& index) const;

    QList<QModelIndex> sourceModelReferenceIndexes(const QModelIndex& index);

public slots:

    void recompute();
    void triggerRecompute();

protected:

    void resetData();
    void computeData();

private:

    class DataAggregationModelPriv;
    DataAggregationModelPriv* const d;
    
};

#endif // DATAAGGREGATIONMODEL_H
