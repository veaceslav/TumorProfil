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

#ifndef DISEASEHISTORYMODEL_H
#define DISEASEHISTORYMODEL_H

// Qt includes

#include <QAbstractItemModel>

// Local includes

#include "diseasehistory.h"

class DiseaseHistoryModel : public QAbstractItemModel
{
    Q_OBJECT
public:

    enum Roles
    {
        HistoryElementRole = Qt::UserRole + 1
    };

    explicit DiseaseHistoryModel(QObject *parent = 0);

    DiseaseHistory history() const;
    void setHistory(const DiseaseHistory& history);

    QModelIndex index(HistoryElement* e, int column = 0) const;
    HistoryElement* element(const QModelIndex& index) const;

    static HistoryElement* retrieveElement(const QModelIndex& index);

    void addElement(HistoryElement* e);
    void addElement(HistoryElement* parent, HistoryElement* e);
    HistoryElement* takeElement(HistoryElement* e);
    void elementChanged(HistoryElement* e);
    
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex& index) const;

signals:
    
public slots:

private:

    DiseaseHistory m_history; 
};

#endif // DISEASEHISTORYMODEL_H
