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

bool CombinedValue::isValid() const
{
    return !resultValue.isNull();
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
    case PathologyPropertyInfo::Comb_HormoneReceptor:
    {
        PathologyPropertyInfo erInfo(PathologyPropertyInfo::IHC_ER);
        PathologyPropertyInfo prInfo(PathologyPropertyInfo::IHC_PR);

        Property erProp = disease.pathologyProperty(erInfo.id);
        Property prProp = disease.pathologyProperty(prInfo.id);

        bool complete = !erProp.isNull() && !prProp.isNull();

        bool er = false, pr = false;

        if (!erProp.isNull())
        {
            ValueTypeCategoryInfo erType(PathologyPropertyInfo::IHC_ER);
            er = erType.toVariantData(erProp).toBool();
        }
        if (!prProp.isNull())
        {
            ValueTypeCategoryInfo prType(PathologyPropertyInfo::IHC_PR);
            pr = prType.toVariantData(prProp).toBool();
        }

        if (er && pr)
        {
            resultValue = true;
            // keep determiningProperty empty, indicating "both"
        }
        else if (!er && !pr)
        {
            if (!complete)
            {
                // can't decide
                resultValue = QVariant();
            }
            else
            {
                resultValue = false;
            }
        }
        else
        {
            resultValue = true;
            if (er)
            {
                determiningProperty = erProp;
            }
            else
            {
                determiningProperty = prProp;
            }
        }
        break;
    }
    case PathologyPropertyInfo::Comb_TripleNegative:
    {
        CombinedValue her2(PathologyPropertyInfo::info(PathologyPropertyInfo::Comb_HER2));
        her2.combine(disease);
        CombinedValue hr(PathologyPropertyInfo::info(PathologyPropertyInfo::Comb_HormoneReceptor));
        hr.combine(disease);
        if (!her2.isValid() || !hr.isValid())
        {
            resultValue = QVariant();
        }
        else
        {
            resultValue = !her2.toValue().toBool() && !hr.toValue().toBool();
        }
        break;
    }
    case PathologyPropertyInfo::Comb_cMetActivation:
    {
        PathologyPropertyInfo ihcInfo(PathologyPropertyInfo::IHC_cMET);
        PathologyPropertyInfo fishInfo(PathologyPropertyInfo::Fish_cMET);

        Property ihc = disease.pathologyProperty(ihcInfo.id);
        Property fish = disease.pathologyProperty(fishInfo.id);

        if (fish.isNull() && ihc.isNull())
        {
            return;
        }

        bool fishPositive = false, hscorePositive = false;
        if (!fish.isNull())
        {
            ValueTypeCategoryInfo fishType(PathologyPropertyInfo::Fish_cMET);
            fishPositive = fishType.toMedicalValue(fish).toBool();
            //qDebug() << "fish" << fishPositive;
        }
        if (!ihc.isNull())
        {
            ValueTypeCategoryInfo ihcType(PathologyPropertyInfo::IHC_cMET);
            //qDebug() << "hscore" << ihcType.toMedicalValue(ihc).value<HScore>().score().toInt() << ihcType.toMedicalValue(ihc).value<HScore>().percentages();
            hscorePositive = (ihcType.toMedicalValue(ihc).value<HScore>().score().toInt() >= 150);
        }

        resultValue = fishPositive || hscorePositive;
        if (hscorePositive)
        {
            determiningProperty = ihc;
        }
        else
        {
            determiningProperty = fish;
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

QVariant CombinedValue::toCombinedVariant() const
{
    /// Returns a variant appropriate for machine sorting
    if (resultValue.isNull())
        return QVariant();
    int id = 0;
    if (resultValue.toBool())
    {
        id |= 1 << 16;
    }
    PathologyPropertyInfo detPropInfo = PathologyPropertyInfo::info(determiningProperty.property);
    ValueTypeCategoryInfo detPropType(detPropInfo);
    id |= detPropInfo.property << 8;
    QVariant propValue = detPropType.toValue(determiningProperty.value);
    id |= (propValue.toInt() & 0xFF);
    qDebug() << toDisplayString() << determiningProperty.property << determiningProperty.value << detPropType.category << propValue << id;
    return id;
}

QString CombinedValue::toDisplayString() const
{
    // "+" or "-"
    ValueTypeCategoryInfo ownType(info);
    QString str = ownType.toDisplayString(result());

    switch (info.property)
    {
    case PathologyPropertyInfo::Comb_HER2:
    {
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
    case PathologyPropertyInfo::Comb_HormoneReceptor:
    {
        if (resultValue.toBool())
        {
            if (determiningProperty.isNull())
            {
                str += " (ER/PR)";
            }
            else if (determiningProperty.property == PathologyPropertyInfo(PathologyPropertyInfo::IHC_ER).id)
            {
                str += " (ER)";
            }
            else
            {
                str += " (PR)";
            }
        }
        return str;
    }
    case PathologyPropertyInfo::Comb_cMetActivation:
    {
        if (resultValue.toBool())
        {
            if (determiningProperty.property == PathologyPropertyInfo(PathologyPropertyInfo::IHC_cMET).id)
            {
                str += " (H-Score)";
            }
            else
            {
                str += " (FISH)";
            }
        }
        return str;
    }
    default:
        return str;
    }
}

