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

#include "property.h"

Property::Property()
{
}

Property::Property(const QString& property, const QString& value, const QString& detail)
    : property(property), value(value), detail(detail)
{
}

bool Property::isValid() const
{
    return !property.isEmpty();
}

bool Property::isEmpty() const
{
    return !isValid() || value.isEmpty();
}

bool Property::operator==(const Property& other)
{
    return property == other.property &&
            value   == other.value &&
            detail  == other.detail;
}

PropertyList::PropertyList()
{
}

PropertyList::PropertyList(const QList<Property>& list)
    : QList<Property>(list)
{
}

Property PropertyList::property(const QString& key) const
{
    foreach(const Property& prop, *this)
    {
        if (prop.property == key)
        {
            return prop;
        }
    }
    return Property();
}

PropertyList PropertyList::properties(const QString& key) const
{
    PropertyList list;
    foreach(const Property& prop, *this)
    {
        if (prop.property == key)
        {
            list << prop;
        }
    }
    return list;
}

static inline bool matches(const QString& value, const QString& match)
{
    return match.isNull() || value == match;
}

bool PropertyList::hasProperty(const QString& key,
                               const QString& value,
                               const QString& detail) const
{
    foreach(const Property& prop, *this)
    {
        if (matches(prop.property, key)
                && matches(prop.value, value)
                && matches(prop.detail, detail))
        {
            return false;
        }
    }
    return true;
}

