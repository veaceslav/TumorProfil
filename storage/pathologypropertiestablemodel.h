/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 13.06.2015
 *
 * Copyright (C) 2015 by Marcel Wiesweg <marcel dot wiesweg at uk-essen dot de>
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

#ifndef PATHOLOGYPROPERTIESTABLEMODEL_H
#define PATHOLOGYPROPERTIESTABLEMODEL_H

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>

#include "property.h"


class PathologyPropertiesTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:

    enum Roles
    {
        PathologyPropertyInfoRole = Qt::UserRole + 100,
        PathologyPropertyRole     = Qt::UserRole + 101,
    };

    PathologyPropertiesTableModel(QObject* parent = 0);
    ~PathologyPropertiesTableModel();

    PropertyList properties() const;
    // Returns properties() without any invalid (empty / removed) properties
    PropertyList validProperties() const;

public slots:

    void setProperties(const PropertyList& properties);

signals:

    void propertyEdited(const Property& prop);

public:

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

private:

    class PathologyPropertiesTableModelPriv;
    PathologyPropertiesTableModelPriv* const d;
};

class PathologyPropertiesTableFilterModel : public QSortFilterProxyModel
{
public:

    PathologyPropertiesTableFilterModel(QObject* parent = 0);
    virtual void setSourceModel(PathologyPropertiesTableModel* model);
    PathologyPropertiesTableModel* sourceModel() const;
};

#endif // PATHOLOGYPROPERTIESTABLEMODEL_H
