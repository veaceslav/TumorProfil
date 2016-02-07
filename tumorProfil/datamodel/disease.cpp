/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 2012-01-22
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

#include <QDebug>

#include "databaseconstants.h"
#include "disease.h"
#include "diseasehistory.h"
#include "pathologypropertyinfo.h"

Disease::Disease()
    : id(0)
{
}

Pathology::Entity Disease::entity() const
{
    foreach (const Pathology& path, pathologies)
    {
        if (path.entity != Pathology::UnknownEntity)
        {
            return path.entity;
        }
    }
    return Pathology::UnknownEntity;
}

bool Disease::hasPathology() const
{
    return !pathologies.isEmpty();
}

Pathology& Disease::firstPathology()
{
    return pathologies.first();
}

const Pathology& Disease::firstPathology() const
{
    return pathologies.first();
}

bool Disease::hasProfilePathology() const
{
    return hasPathology(PathologyContextInfo::Tumorprofil);
}

Pathology& Disease::firstProfilePathology()
{
    return firstPathology(PathologyContextInfo::Tumorprofil);
}

const Pathology& Disease::firstProfilePathology() const
{
    return firstPathology(PathologyContextInfo::Tumorprofil);
}

bool Disease::hasPathology(int context) const
{
    return hasPathology(PathologyContextInfo(PathologyContextInfo::Context(context)).id);
}

Pathology& Disease::firstPathology(int context)
{
    return firstPathology(PathologyContextInfo(PathologyContextInfo::Context(context)).id);
}

const Pathology& Disease::firstPathology(int context) const
{
    return firstPathology(PathologyContextInfo(PathologyContextInfo::Context(context)).id);
}

bool Disease::hasPathology(const QString& id) const
{
    foreach (const Pathology& path, pathologies)
    {
        if (path.context == id)
        {
            return true;
        }
    }
    return false;
}

Pathology& Disease::firstPathology(const QString& id)
{
    QList<Pathology>::iterator it;
    for (it=pathologies.begin(); it != pathologies.end(); ++it)
    {
        if (it->context == id)
        {
            return *it;
        }
    }
    return *it; // crashes
}

const Pathology& Disease::firstPathology(const QString& id) const
{
    QList<Pathology>::const_iterator it;
    for (it=pathologies.begin(); it != pathologies.end(); ++it)
    {
        if (it->context == id)
        {
            return *it;
        }
    }
    return *it; // crashes
}

Property Disease::pathologyProperty(const QString& prop) const
{
    foreach (const Pathology& path, pathologies)
    {
        Property p = path.properties.property(prop);
        if (p.isValid())
        {
            return p;
        }
    }
    return Property();
}

PropertyList Disease::pathologyProperties(const QString& prop) const
{
    PropertyList props;
    foreach (const Pathology& path, pathologies)
    {
        props += path.properties.properties(prop);
    }
    return props;
}

bool Disease::hasPathologyProperty(const QString& prop, const QString& value, const QString& detail) const
{
    foreach (const Pathology& path, pathologies)
    {
        if (path.properties.hasProperty(prop, value, detail))
        {
            return true;
        }
    }
    return false;
}

PropertyList Disease::allPathologyProperties() const
{
    if (pathologies.size() == 1)
    {
        return pathologies.first().properties;
    }

    PropertyList props;
    foreach (const Pathology& path, pathologies)
    {
        props += path.properties;
    }
    return props;
}

DiseaseHistory Disease::history() const
{
    Property prop = diseaseProperties.property(DiseasePropertyName::diseaseHistory());
    return DiseaseHistory::fromXml(prop.value);
}

void Disease::setHistory(const DiseaseHistory& history)
{
    diseaseProperties.setProperty(DiseasePropertyName::diseaseHistory(), history.toXml());
}
