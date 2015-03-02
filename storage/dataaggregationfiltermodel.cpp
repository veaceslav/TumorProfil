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

#include "dataaggregationfiltermodel.h"

#include <QDebug>

DataAggregationFilterModel::DataAggregationFilterModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
}

bool DataAggregationFilterModel::filterAcceptsColumn(int source_column,
                                                     const QModelIndex &) const
{
    const int rowCount = sourceModel()->rowCount();
    for (int row=0; row<rowCount; row++)
    {
        QModelIndex index = sourceModel()->index(row, source_column);
        if (!index.data().isNull())
        {
            return true;
        }
    }
    return false;
}

bool DataAggregationFilterModel::filterAcceptsRow(int,
                                                  const QModelIndex &) const
{
    return true;
}
