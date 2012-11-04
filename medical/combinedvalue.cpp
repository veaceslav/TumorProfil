/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 03.11.2012
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

// Qt includes

#include <QDebug>

// Local includes

#include "combinedvalue.h"
#include "ihcscore.h"

CombinedValue::CombinedValue(const PathologyPropertyInfo& info)
    : info(info)
{
}

Property CombinedValue::combine(Patient::Ptr p)
{
    if (!p->hasDisease())
    {
        return Property();
    }
    QVariant returnValue;
    switch (info.property)
    {
    case PathologyPropertyInfo::Comb_HER2:
    {
        PathologyPropertyInfo ihcInfo(PathologyPropertyInfo::IHC_HER2);
        PathologyPropertyInfo fishInfo(PathologyPropertyInfo::Fish_HER2);

        Property ihc = p->firstDisease().pathologyProperty(ihcInfo.id);
        Property fish = p->firstDisease().pathologyProperty(fishInfo.id);

        // If we have FISH, it has precedence and decides yes/no
        if (!fish.isNull())
        {
            ValueTypeCategoryInfo fishType(PathologyPropertyInfo::Fish_HER2);
            QVariant value = fishType.toMedicalValue(fish);
            break;
        }
        ValueTypeCategoryInfo ihcType(PathologyPropertyInfo::IHC_HER2);
        IHCScore score = ihcType.toIHCScore(ihc);
        if (score.isValid())
        {
            returnValue = (score.score().toInt() >= 7);
        }
        break;
    }
    default:
        qDebug() << "Unsupported combined value" << info.id << info.property;
        return Property();
    }

    ValueTypeCategoryInfo ownType(info);
    return Property(info.id, ownType.toPropertyValue(returnValue));
}
