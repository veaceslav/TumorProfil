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

#include "pathology.h"

#include <QDebug>

Pathology::Pathology()
    : entity(UnknownEntity),
      sampleOrigin(UnknownOrigin),
      id(0)
{
}

bool Pathology::operator==(const Pathology& other) const
{
    if (   entity       == other.entity
        && sampleOrigin == other.sampleOrigin
        && context      == other.context
        && date         == other.date
           )
    {
        if (properties == other.properties)
        {
            return true;
        }
        if (properties.size() != other.properties.size())
        {
            //return false;
        }
        // Check if only order is different
        PropertyList leftSorted(properties);
        qSort(leftSorted);
        PropertyList rightSorted(properties);
        qSort(rightSorted);
        if (leftSorted == rightSorted)
        {
            return true;
        }
        qDebug() << "Left properties size" << properties.size();
        foreach (const Property& prop, properties)
        {
            qDebug() << prop.property << prop.value << prop.detail;
        }
        qDebug() << "Right properties size" << other.properties.size();
        foreach (const Property& prop, other.properties)
        {
            qDebug() << prop.property << prop.value << prop.detail;
        }
    }
    return false;
}
