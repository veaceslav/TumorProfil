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
