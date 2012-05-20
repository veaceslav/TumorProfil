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

#ifndef AGGREGATETABLEVIEW_H
#define AGGREGATETABLEVIEW_H

#include "analysistableview.h"

class DataAggregationModel;

class AggregateTableView : public AnalysisTableView
{
    Q_OBJECT
public:

    explicit AggregateTableView(QWidget *parent = 0);
    ~AggregateTableView();
    
    void setSourceModel(QAbstractItemModel* source);

    DataAggregationModel* dataAggregationModel() const;

signals:

    void activatedReferenceIndexes(const QList<QModelIndex>&);
    
public slots:

protected slots:

    void slotActivated(const QModelIndex&);

private:

    class AggregateTableViewPriv;
    AggregateTableViewPriv* const d;
    
};

#endif // AGGREGATETABLEVIEW_H
