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

#include "pathologypropertiestablemodel.h"

#include <QDebug>

#include "pathologypropertyinfo.h"

class PathologyPropertiesTableModel::PathologyPropertiesTableModelPriv
{
public:

    PropertyList properties;

    bool hasInvalidProperties() const
    {
        foreach (const Property& prop, properties)
        {
            if (!prop.isValid())
            {
                return true;
            }
        }
        return false;
    }
};

namespace {
    enum Columns
    {
        PropertyNameColumn  = 0,
        PropertyValueColumn = 1,
        ColumnCount         = 2
    };
}

PathologyPropertiesTableModel::PathologyPropertiesTableModel(QObject* parent)
    : QAbstractTableModel(parent),
      d(new PathologyPropertiesTableModelPriv)
{
}

PathologyPropertiesTableModel::~PathologyPropertiesTableModel()
{
    delete d;
}

PropertyList PathologyPropertiesTableModel::properties() const
{
    return d->properties;
}

PropertyList PathologyPropertiesTableModel::validProperties() const
{
    if (!d->hasInvalidProperties())
    {
        return d->properties;
    }
    PropertyList props;
    foreach (const Property& prop, d->properties)
    {
        if (prop.isValid())
        {
            props << prop;
        }
    }
    return props;
}

void PathologyPropertiesTableModel::setProperties(const PropertyList& properties)
{
    beginResetModel();
    d->properties = properties;
    endResetModel();
}

QVariant PathologyPropertiesTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    const Property& prop = d->properties[index.row()];
    PathologyPropertyInfo info = PathologyPropertyInfo::info(prop.property);
    ValueTypeCategoryInfo catInfo(info);

    switch (role)
    {
    case Qt::DisplayRole:
        switch (index.column()) {
        case PropertyNameColumn:
            return info.label;
        case PropertyValueColumn:
            return catInfo.toLongDisplayString(prop);
        }
    case Qt::EditRole:
        if (index.column() == PropertyValueColumn)
        {
            return QVariant::fromValue(d->properties[index.row()]);
        }
    case Qt::DecorationRole:
        break;
    case PathologyPropertyRole:
        return QVariant::fromValue(prop);
    case PathologyPropertyInfoRole:
        return QVariant::fromValue(info);
    }

    return QVariant();
}

QVariant PathologyPropertiesTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case PropertyNameColumn:
            return tr("Befund");
        case PropertyValueColumn:
            return tr("Ergebnis");
        }
    }
    return QVariant();
}

int PathologyPropertiesTableModel::rowCount(const QModelIndex&) const
{
    return d->properties.size();
}

int PathologyPropertiesTableModel::columnCount(const QModelIndex&) const
{
    return ColumnCount;
}

Qt::ItemFlags PathologyPropertiesTableModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;
    if (index.column() == PropertyValueColumn)
    {
        flags |= Qt::ItemIsEditable;
    }
    return flags;
}

bool PathologyPropertiesTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole || index.column() != PropertyValueColumn)
    {
        return false;
    }
    Property newProp = value.value<Property>();
    qDebug() << "setData" << value << newProp.property;
    d->properties[index.row()] = newProp;
    emit dataChanged(index, index);
    return true;
}

PathologyPropertiesTableFilterModel::PathologyPropertiesTableFilterModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
}

void PathologyPropertiesTableFilterModel::setSourceModel(PathologyPropertiesTableModel* model)
{
    QSortFilterProxyModel::setSourceModel(model);
}

PathologyPropertiesTableModel* PathologyPropertiesTableFilterModel::sourceModel() const
{
    return static_cast<PathologyPropertiesTableModel*>(QSortFilterProxyModel::sourceModel());
}

