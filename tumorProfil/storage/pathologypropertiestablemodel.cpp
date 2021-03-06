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
#include <QPair>

#include "pathologypropertyinfo.h"

template <class ListType, class IteratorType, class PropertyType>
PropertyType& segmentedPropertiesListElement(ListType& list, int index)
{
    for (IteratorType it = list.begin(); /* stopped by index, or crash if index is invalid*/; ++it)
    {
        const int propertiesSize = it->properties.size();
        if (index < propertiesSize)
        {
            return it->properties[index];
        }
        index -= propertiesSize;
    }
}

template <class ListType, class IteratorType, class PropertyType>
IteratorType segmentedPropertiesListSegmentIterator(ListType& list, int index)
{
    return segmentedPropertiesListSegmentIterator<ListType, IteratorType, PropertyType>(list, &index);
}

// Note: This method takes the index parameter as a pointer and changes it
// to contain the index of the property in the Pathology's property list!
template <class ListType, class IteratorType, class PropertyType>
IteratorType segmentedPropertiesListSegmentIterator(ListType& list, int* index)
{
    for (IteratorType it = list.begin(); /* stopped by index, or crash if index is invalid*/; ++it)
    {
        const int propertiesSize = it->properties.size();
        if (*index < propertiesSize)
        {
            return it;
        }
        *index -= propertiesSize;
    }
}


class PathologyPropertiesTableModel::PathologyPropertiesTableModelPriv
{
public:

    QList<Pathology> pathologies;

    Property& property(int index)
    {
        return segmentedPropertiesListElement<QList<Pathology>, QList<Pathology>::iterator, Property>(pathologies, index);
    }
    const Property& property(int index) const
    {
        return segmentedPropertiesListElement<const QList<Pathology>, QList<Pathology>::const_iterator, const Property>(pathologies, index);
    }

    Pathology& pathology(int index)
    {
        return *segmentedPropertiesListSegmentIterator<QList<Pathology>, QList<Pathology>::iterator, Property>(pathologies, index);
    }
    const Pathology& pathology(int index) const
    {
        return *segmentedPropertiesListSegmentIterator<const QList<Pathology>, QList<Pathology>::const_iterator, const Property>(pathologies, index);
    }

    // Converts from "flat" index over all pathologies, to index in pathologies / index in the pathology's property list
    int toOverallIndex(int pathologyIndex, int propertyIndex)
    {
        int index = 0;
        for (int i=0; i<pathologyIndex; ++i)
        {
            index += pathologies[i].properties.size();
        }
        return index + propertyIndex;
    }
    QPair<int,int> fromOverallIndex(int overallIndex)
    {
        int index = overallIndex;
        QList<Pathology>::const_iterator pathIterator = segmentedPropertiesListSegmentIterator<const QList<Pathology>, QList<Pathology>::const_iterator, const Property>(pathologies, &index);
        return QPair<int,int>(pathIterator-pathologies.begin(), index);
    }

    int propertiesSize() const
    {
        int size = 0;
        for (QList<Pathology>::const_iterator it = pathologies.cbegin(); it != pathologies.end(); ++it)
        {
            size += it->properties.size();
        }
        return size;
    }

    bool         editingEnabled;

    bool pathologyListClean() const
    {
        foreach (const Pathology& pathology, pathologies)
        {
            if (pathology.properties.isEmpty() && pathology.reports.isEmpty())
            {
                return true;
            }
            foreach (const Property& prop, pathology.properties)
            {
                if (!prop.isValid())
                {
                    return true;
                }
            }
        }
        return false;
    }
};

namespace {
    enum Columns
    {
        PropertyNameColumn,
        PathologyDateColumn,
        PropertyValueColumn,
        ColumnCount
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

QList<Pathology> PathologyPropertiesTableModel::pathologies() const
{
    return d->pathologies;
}

QList<Pathology> PathologyPropertiesTableModel::pathologiesConsolidated() const
{
    if (!d->pathologyListClean())
    {
        return d->pathologies;
    }
    QList<Pathology> cleanPaths;
    foreach (const Pathology& pathology, d->pathologies)
    {
        if (pathology.properties.isEmpty() && pathology.reports.isEmpty())
        {
            continue;
        }
        cleanPaths << pathology;
        cleanPaths.last().properties.clear();
        foreach (const Property& prop, pathology.properties)
        {
            if (prop.isValid())
            {
                cleanPaths.last().properties << prop;
            }
        }
    }
    return cleanPaths;
}

void PathologyPropertiesTableModel::setPathology(const Pathology& pathology)
{
    QList<Pathology> paths;
    paths << pathology;
    setPathologies(paths);
}

void PathologyPropertiesTableModel::setPathologies(const QList<Pathology>& pathologies)
{
    beginResetModel();
    d->pathologies = pathologies;
    endResetModel();
}

QVariant PathologyPropertiesTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    const Property& prop = d->property(index.row());
    PathologyPropertyInfo info = PathologyPropertyInfo::info(prop.property);
    ValueTypeCategoryInfo catInfo(info);

    switch (role)
    {
    case Qt::DisplayRole:
        switch (index.column()) {
        case PropertyNameColumn:
            return info.label;
        case PathologyDateColumn:
        {
            const QDate& date = d->pathology(index.row()).date;
            if (date.isValid())
            {
                return date;
            }
            return QVariant();
        }
        case PropertyValueColumn:
            return catInfo.toLongDisplayString(prop);
        }
    case Qt::EditRole:
        if (index.column() == PropertyValueColumn)
        {
            return QVariant::fromValue(d->property(index.row()));
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
        case PathologyDateColumn:
            return tr("Datum");
        case PropertyValueColumn:
            return tr("Ergebnis");
        }
    }
    return QVariant();
}

int PathologyPropertiesTableModel::rowCount(const QModelIndex&) const
{
    return d->propertiesSize();
}

int PathologyPropertiesTableModel::columnCount(const QModelIndex&) const
{
    return ColumnCount;
}

Qt::ItemFlags PathologyPropertiesTableModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;
    if (d->editingEnabled && index.column() == PropertyValueColumn)
    {
        flags |= Qt::ItemIsEditable;
    }
    return flags;
}

bool PathologyPropertiesTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!d->editingEnabled || !index.isValid() || role != Qt::EditRole || index.column() != PropertyValueColumn)
    {
        return false;
    }
    Property newProp = value.value<Property>();
    Property& oldProp = d->property(index.row());

    if (newProp.isEmpty())
    {
        beginRemoveRows(QModelIndex(), index.row(), index.row());
        QPair<int,int> indexes = d->fromOverallIndex(index.row());
        d->pathologies[indexes.first].properties.removeAt(indexes.second);
        endInsertRows();
        return true;
    }

    if (oldProp == newProp)
    {
        return true;
    }

    Property& propRef = d->property(index.row());
    propRef = newProp;
    emit dataChanged(index, index);
    emit propertyEdited(d->pathology(index.row()), propRef);
    return true;
}

void PathologyPropertiesTableModel::setEditingEnabled(bool enabled)
{
    if (d->editingEnabled == enabled)
    {
        return;
    }
    d->editingEnabled = enabled;
    if (rowCount())
    {
        emit dataChanged(index(0,0), index(rowCount()-1, columnCount()-1));
    }
}

int PathologyPropertiesTableModel::addPathology(const Pathology& pathology)
{
    beginInsertRows(QModelIndex(), d->propertiesSize(), d->propertiesSize() + pathology.properties.size());
    d->pathologies << pathology;
    endInsertRows();
    return d->pathologies.size() - 1;
}

void PathologyPropertiesTableModel::addProperties(const PropertyList& properties)
{
    if (d->pathologies.isEmpty())
    {
        // No pathology yet? Create a default one
        Pathology path;
        path.properties = properties;
        addPathology(path);
    }
    else
    {
        addProperties(d->pathologies.size()-1, properties);
    }
}

void PathologyPropertiesTableModel::addProperties(int pathologyIndex, const PropertyList& properties)
{
    Pathology& path  = d->pathologies[pathologyIndex];
    int firstNewIndex = d->toOverallIndex(pathologyIndex, 0) + path.properties.size();
    beginInsertRows(QModelIndex(), firstNewIndex, firstNewIndex + properties.size());
    path.properties << properties;
    endInsertRows();
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
