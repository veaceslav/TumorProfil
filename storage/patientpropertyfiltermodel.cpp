/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 21.02.2012
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

#include "patientpropertyfiltermodel.h"
#include "patientmodel.h"
#include "pathologypropertyinfo.h"

class PatientPropertyFilterModel::PatientPropertyFilterModelPriv
{
public:
    PatientPropertyFilterModelPriv()
    {
    }

    PatientPropertyFilterSettings settings;
};

PatientPropertyFilterModel::PatientPropertyFilterModel(QObject* parent)
    : QSortFilterProxyModel(parent),
      d(new PatientPropertyFilterModelPriv)
{
    setSortRole(PatientModel::VariantDataRole);
}

PatientPropertyFilterModel::~PatientPropertyFilterModel()
{
    delete d;
}

PatientPropertyFilterSettings PatientPropertyFilterModel::filterSettings() const
{
    return d->settings;
}

void PatientPropertyFilterModel::setFilterSettings(const PatientPropertyFilterSettings& settings)
{
    d->settings = settings;
    invalidateFilter();
}

void PatientPropertyFilterModel::filterByEntity(Pathology::Entity entity)
{
    PatientPropertyFilterSettings settings;
    settings.entities << entity;
    setFilterSettings(settings);
}

void PatientPropertyFilterModel::filterByPathologyProperty(const QString& property, const QVariant& value)
{
    PatientPropertyFilterSettings settings;
    settings.pathologyProperties[property] = value;
    setFilterSettings(settings);
}

bool PatientPropertyFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    Patient::Ptr p = PatientModel::retrievePatient(index);
    if (!p)
    {
        return false;
    }

    const bool hasPathology = p->hasPathology();
    if (!d->settings.entities.isEmpty())
    {
        if (!hasPathology)
        {
            return false;
        }
        bool hasMatch = false;
        foreach (const Pathology& path, p->firstDisease().pathologies)
        {
            if (d->settings.entities.contains(path.entity))
            {
                hasMatch = true;
                break;
            }
        }

        if (!hasMatch)
        {
            return false;
        }
    }

    if (!d->settings.pathologyProperties.isEmpty())
    {
        if (!hasPathology)
        {
            return false;
        }

        QMap<QString,QVariant>::const_iterator it;
        for (it = d->settings.pathologyProperties.begin();
             it != d->settings.pathologyProperties.end(); ++it)
        {
            bool hasMatch = false;
            foreach (const Pathology& path, p->firstDisease().pathologies)
            {
                if (it.value().type() == QVariant::String)
                {
                    hasMatch = path.properties.hasProperty(it.key(), it.value().toString());
                }
                else
                {
                    Property prop = path.properties.property(it.key());
                    if (prop.isValid())
                    {
                        PathologyPropertyInfo info = PathologyPropertyInfo::info(prop.property);
                        hasMatch = (it.value() ==
                                    ValueTypeCategoryInfo(info.valueType).toValue(prop.value));
                    }
                }

                if (hasMatch)
                {
                    break;
                }
            }

            if (!hasMatch)
            {
                return false;
            }
        }
    }
    return true;
}
