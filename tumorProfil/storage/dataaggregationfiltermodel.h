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

#ifndef DATAAGGREGATIONFILTERMODEL_H
#define DATAAGGREGATIONFILTERMODEL_H

#include <QSortFilterProxyModel>

class DataAggregationFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit DataAggregationFilterModel(QObject *parent = 0);

    virtual bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const;
    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

signals:
    
public slots:
    
};

#endif // DATAAGGREGATIONFILTERMODEL_H
