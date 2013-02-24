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

void CombinedValue::combine(const Disease& disease)
{
    resultValue = QVariant();
    determiningProperty = Property();
    switch (info.property)
    {
    case PathologyPropertyInfo::Comb_HER2:
    {
        PathologyPropertyInfo ihcInfo(PathologyPropertyInfo::IHC_HER2_DAKO);
        PathologyPropertyInfo fishInfo(PathologyPropertyInfo::Fish_HER2);

        Property ihc = disease.pathologyProperty(ihcInfo.id);
        Property fish = disease.pathologyProperty(fishInfo.id);

        if (fish.isNull() && ihc.isNull())
        {
            return;
        }

        // If we have FISH, it has precedence and decides yes/no
        if (!fish.isNull())
        {
            ValueTypeCategoryInfo fishType(PathologyPropertyInfo::Fish_HER2);
            determiningProperty = fish;
            resultValue = fishType.toMedicalValue(fish);
            break;
        }
        if (!ihc.isNull())
        {
            ValueTypeCategoryInfo ihcType(PathologyPropertyInfo::IHC_HER2_DAKO);
            determiningProperty = ihc;
            resultValue = (ihc.value.toInt() == 3);
        }
        break;
    }
    default:
        qDebug() << "Unsupported combined value" << info.id << info.property;
        break;
    }
}

Property CombinedValue::result() const
{
    if (!resultValue.isNull())
    {
        ValueTypeCategoryInfo ownType(info);
        return Property(info.id, ownType.toPropertyValue(resultValue));
    }
    return Property();
}

QVariant CombinedValue::toValue() const
{
    return resultValue;
}

QString CombinedValue::toDisplayString() const
{
    switch (info.property)
    {
    case PathologyPropertyInfo::Comb_HER2:
    {
        // "+" or "-"
        ValueTypeCategoryInfo ownType(info);
        QString str = ownType.toDisplayString(result());

        // "1+" etc. org "+"/"-"
        ValueTypeCategoryInfo detInfoType(PathologyPropertyInfo::info(determiningProperty.property));
        QString propStr = detInfoType.toDisplayString(determiningProperty);

        if (determiningProperty.property == PathologyPropertyInfo(PathologyPropertyInfo::IHC_HER2_DAKO).id)
        {
            str += " (IHC ";
        }
        else
        {
            str += " (FISH ";
        }
        str += propStr;
        str += ")";
        return str;
    }
    default:
        return QString();
    }
}

