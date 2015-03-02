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

#include <QDebug>

#include "combinedvalue.h"
#include "databaseconstants.h"
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
    filterByEntity(QList<Pathology::Entity>() << entity);
}

void PatientPropertyFilterModel::filterByEntity(QList<Pathology::Entity> entities)
{
    PatientPropertyFilterSettings settings = d->settings;
    settings.entities = entities;
    settings.pathologyProperties.clear();
    setFilterSettings(settings);
}

void PatientPropertyFilterModel::filterByPathologyProperty(const QMap<QString, QVariant>& filter)
{
    PatientPropertyFilterSettings settings = d->settings;
    settings.entities.clear();
    settings.pathologyProperties = filter;
    setFilterSettings(settings);
}

void PatientPropertyFilterModel::filterByPathologyProperty(const QString& property, const QVariant& value)
{
    PatientPropertyFilterSettings settings = d->settings;
    settings.entities.clear();
    settings.pathologyProperties.clear();
    settings.pathologyProperties[property] = value;
    setFilterSettings(settings);
}

void PatientPropertyFilterModel::filterByPathologyContext(const QString& property, bool value)
{
    PatientPropertyFilterSettings settings;
    settings.pathologyContexts[property] = value;
    setFilterSettings(settings);
}

void PatientPropertyFilterModel::filterByTrialParticipation(const QString& property, bool value)
{
    PatientPropertyFilterSettings settings = d->settings;
    settings.trialParticipation.clear();
    settings.trialParticipation[property] = value;
    setFilterSettings(settings);
}

bool PatientPropertyFilterSettings::matchesEntities(Patient::Ptr p) const
{
    foreach (const Pathology& path, p->firstDisease().pathologies)
    {
        if (entities.contains(path.entity))
        {
            return true;
        }
    }
    return false;
}

bool PatientPropertyFilterSettings::matchesPathologyProperties(Patient::Ptr p,
                                                               const QMap<QString, QVariant>& pathProps,
                                                               Logic logic) const
{
    bool hasMatch = false;
    QMap<QString,QVariant>::const_iterator it;
    for (it = pathProps.begin();
         it != pathProps.end(); ++it)
    {
        const Disease& disease = p->firstDisease();
        PathologyPropertyInfo info = PathologyPropertyInfo::info(it.key());
        if (info.isCombined())
        {
            CombinedValue combinedValue(info);
            combinedValue.combine(p->firstDisease());
            qDebug() << it.key() << combinedValue.toDisplayString() << combinedValue.toValue();
            hasMatch = (it.value() == combinedValue.toValue());
        }
        else
        {
            foreach (const Pathology& path, disease.pathologies)
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
                        QVariant propValue = ValueTypeCategoryInfo(info.valueType).toValue(prop.value);
                        if (it.value().type() == QVariant::Bool && propValue.type() != QVariant::Bool)
                        {
                            // This allows to simplify IHC scores to boolean comparison
                            hasMatch = (it.value().toBool() == propValue.toBool());
                        }
                        else
                        {
                            hasMatch = (it.value() == propValue);
                        }
                    }
                }

                if (hasMatch)
                {
                    break;
                }
            }
        }

        switch (logic)
        {
        case And:
            if (!hasMatch)
            {
                return false;
            }
            break;
        case Or:
            if (hasMatch)
            {
                return true;
            }
            break;
        }

    }

    switch (logic)
    {
    case And:
        return true;
    case Or:
    default:
        return false;
    }
}

bool PatientPropertyFilterSettings::matchesPathologyProperties(Patient::Ptr p) const
{
    return matchesPathologyProperties(p, pathologyProperties, Or);
}

bool PatientPropertyFilterSettings::matchesPathologyPropertiesAnd(Patient::Ptr p) const
{
    return matchesPathologyProperties(p, pathologyPropertiesAnd, And);
}

bool PatientPropertyFilterSettings::matchesPathologyContexts(Patient::Ptr p) const
{
    QMap<QString,bool>::const_iterator it;
    for (it = pathologyContexts.begin();
         it != pathologyContexts.end(); ++it)
    {
        foreach (const Pathology& path, p->firstDisease().pathologies)
        {
            if (path.context == it.key())
            {
                // Searching for "NOT"?
                if (it.value() == false)
                {
                    return false;
                }
                return true;
            }
        }
    }
    return false;
}

bool PatientPropertyFilterSettings::matchesTrialParticipation(Patient::Ptr p) const
{
    QMap<QString,bool>::const_iterator it;
    for (it = trialParticipation.begin();
         it != trialParticipation.end(); ++it)
    {
        return it.value() ==
                    p->patientProperties.hasProperty(PatientPropertyName::trialParticipation(), it.key());
    }
    return false;
}

static bool hasMatchingResultDate(const Disease& disease, const QDate& date, int compareTarget,
                                  const QMap<QString, bool>& pathologyContexts)
{
    foreach (const Pathology& path, disease.pathologies)
    {
        if (!pathologyContexts.isEmpty())
        {
            QMap<QString, bool>::const_iterator it = pathologyContexts.find(path.context);
            if (it == pathologyContexts.end() || it.value() == false)
            {
                continue;
            }
        }

        if (compareTarget == -1 && path.date < date)
        {
            return true;
        }
        if (compareTarget == 1 && path.date > date)
        {
            return true;
        }
    }
    return false;
}

bool PatientPropertyFilterSettings::matchesDates(Patient::Ptr p) const
{
    const Disease& disease = p->firstDisease();
    QMap<QString, bool> filterContexts;
    if(dateAppliesCombinedWithContext)
    {
        filterContexts = pathologyContexts;
    }

    if (resultDateBegin.isValid())
    {
        if (!hasMatchingResultDate(disease, resultDateBegin, 1, filterContexts))
        {
            return false;
        }
    }
    if (resultDateBegin.isValid())
    {
        if (!hasMatchingResultDate(disease, resultDateEnd, -1, filterContexts))
        {
            return false;
        }
    }
    return true;
}

bool PatientPropertyFilterSettings::matchesCriteria(Patient::Ptr p) const
{
    QMap<Criteria,bool>::const_iterator it;
    for (it = criteria.begin(); it != criteria.end(); ++it)
    {
        switch (it.key())
        {
        case LocalCenterOrigin:
        {
            bool isFromLocalCenter = true;
            if (p->surname.startsWith("Dktk", Qt::CaseInsensitive) && p->firstName.trimmed().isEmpty())
            {
                isFromLocalCenter = false;
            }
            if (isFromLocalCenter != it.value())
            {
                return false;
            }
        }
        }
    }
    return true;
}

bool PatientPropertyFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    Patient::Ptr p = PatientModel::retrievePatient(index);
    if (!p)
    {
        return false;
    }

    const bool filteringByEntity       = !d->settings.entities.isEmpty();
    const bool filteringByPathology    = !d->settings.pathologyProperties.isEmpty();
    const bool filteringByPathologyAnd = !d->settings.pathologyPropertiesAnd.isEmpty();
    const bool filteringByContext      = !d->settings.pathologyContexts.isEmpty();
    const bool filteringByTrial        = !d->settings.trialParticipation.isEmpty();
    const bool filteringByDate         = d->settings.resultDateBegin.isValid() ||
                                           d->settings.resultDateEnd.isValid();
    const bool filteringByCriteria     = !d->settings.criteria.isEmpty();

    if (!filteringByEntity && !filteringByPathology && !filteringByPathologyAnd && !filteringByContext && !filteringByTrial && !filteringByDate && !filteringByCriteria)
    {
        return true;
    }
    else
    {
        if (!p->hasPathology() && (filteringByPathology || filteringByPathologyAnd || filteringByContext || filteringByDate))
        {
            return false;
        }
    }

    if (filteringByContext)
    {
        if (!d->settings.matchesPathologyContexts(p))
        {
            return false;
        }
    }

    if (filteringByTrial)
    {
        if (!d->settings.matchesTrialParticipation(p))
        {
            return false;
        }
    }

    if (filteringByCriteria)
    {
        if (!d->settings.matchesCriteria(p))
        {
            return false;
        }
    }

    if (filteringByDate)
    {
        if (!d->settings.matchesDates(p))
        {
            return false;
        }
    }

    if (filteringByEntity)
    {
        if (!d->settings.matchesEntities(p))
        {
            return false;
        }
    }

    if (filteringByPathology)
    {
        if (!d->settings.matchesPathologyProperties(p))
        {
            return false;
        }
    }

    if (filteringByPathologyAnd)
    {
        if (!d->settings.matchesPathologyPropertiesAnd(p))
        {
            return false;
        }
    }

    return true;
}
