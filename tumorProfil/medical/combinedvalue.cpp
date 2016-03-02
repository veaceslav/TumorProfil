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
#include <QLocale>
#include <QRegExp>

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

static float textCENRatioToFloat(const QString& ratio)
{
    if (ratio.isEmpty())
    {
        return 0;
    }
    QString s(ratio);
    s.remove(QRegExp("\\(.*\\)"));
    s = s.trimmed();
    if (s.startsWith('<'))
    {
        s.remove(0, 1);
    }
    bool ok = false;
    QLocale locale;
    float f = locale.toFloat(s, &ok);
    if (ok)
    {
        return f;
    }
    qDebug() << "Invalid FISH ratio" << ratio << "processed" << s;
    return 0;
}

QVariant CombinedValue::fishResult(const Disease& disease)
{
    switch (info.property)
    {
    case PathologyPropertyInfo::Comb_HER2:
    {
        PathologyPropertyInfo fishInfo(PathologyPropertyInfo::Fish_HER2);
        Property fish = disease.pathologyProperty(fishInfo.id);
        return fishResult(disease, fish);
    }
    case PathologyPropertyInfo::Comb_cMetActivation:
    {
        PathologyPropertyInfo fishInfo(PathologyPropertyInfo::Fish_cMET);
        Property fish = disease.pathologyProperty(fishInfo.id);
        return fishResult(disease, fish);
    }
    default:
        qDebug() << "CombinedValue::fishResult(): " << info.plainTextLabel() << "not supported";
    }
    return QVariant();
}

QVariant CombinedValue::fishResult(const Disease& disease, const Property& prop)
{
    if (prop.isNull())
    {
        return QVariant();
    }
    switch (info.property)
    {
    case PathologyPropertyInfo::Comb_HER2:
    {
        PathologyPropertyInfo fishInfo(PathologyPropertyInfo::Fish_HER2);
        ValueTypeCategoryInfo fishType(fishInfo);
        QVariant result = fishType.toMedicalValue(prop);
        if (!result.toBool())
        {
            if (disease.entity() == Pathology::PulmonaryAdeno
                    || disease.entity() == Pathology::PulmonaryAdenosquamous
                    || disease.entity() == Pathology::PulmonaryBronchoalveloar)
            {
                // here we regard Her2 as positive if Her2/CEN >= 2.0, not 2.2
                if (textCENRatioToFloat(prop.detail) >= 2.0)
                {
                    result = true;
                }
            }
        }
        return result;
    }
    case PathologyPropertyInfo::Comb_cMetActivation:
    {
        ValueTypeCategoryInfo fishType(PathologyPropertyInfo::Fish_cMET);
        return fishType.toMedicalValue(prop).toBool();
    }
    default:
        qDebug() << "CombinedValue::fishResult: " << info.plainTextLabel() << "not supported";
    }
    return QVariant();
}

void CombinedValue::listOfMutationsResult(const Disease& disease, const QList<PathologyPropertyInfo::Property>& propIds)
{
    int sawMutation = 0;
    foreach (PathologyPropertyInfo::Property propertyId, propIds)
    {
        PathologyPropertyInfo info(propertyId);
        Property mut = disease.pathologyProperty(info.id);
        if (mut.isNull())
        {
            continue;
        }
        ValueTypeCategoryInfo mutType(propertyId);
        if (mutType.toMedicalValue(mut).toBool())
        {
            resultValue = true;
            determiningProperty = mut;
            break;
        }
        sawMutation++;
    }
    // If we did not see a mutation, but saw all mutation analyses as wildtype, result is valid.
    // Result is not valid if we did not see all for this combined value.
    if (resultValue.isNull() && sawMutation == propIds.size())
    {
        resultValue = false;
    }
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

        bool fishPositive = fishResult(disease, fish).toBool();
        bool ihcPositive  = (ihc.value.toInt() == 3);
        if (fishPositive || ihcPositive)
        {
            resultValue = true;
            determiningProperty = fishPositive ? fish : ihc;
        }
        else
        {
            resultValue = false;
            determiningProperty = fish.isNull() ? ihc : fish;
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
            fishPositive = fishResult(disease, fish).toBool();
            //qDebug() << "fish" << fishPositive;
        }
        if (!ihc.isNull())
        {
            ValueTypeCategoryInfo ihcType(PathologyPropertyInfo::IHC_cMET);
            //qDebug() << "hscore" << ihcType.toMedicalValue(ihc).value<HScore>().score().toInt() << ihcType.toMedicalValue(ihc).value<HScore>().percentages();
            hscorePositive = (ihcType.toMedicalValue(ihc).value<HScore>().score().toInt() >= 250);
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
    case PathologyPropertyInfo::Comb_cMetIHC3plusScore:
    {
        PathologyPropertyInfo ihcInfo(PathologyPropertyInfo::IHC_cMET);
        Property ihc = disease.pathologyProperty(ihcInfo.id);

        if (ihc.isNull())
        {
            return;
        }
        ValueTypeCategoryInfo ihcType(PathologyPropertyInfo::IHC_cMET);
        // CINC280X2102 definition
        resultValue = (ihcType.toMedicalValue(ihc).value<HScore>().percentageStrong() >= 50);
        determiningProperty = ihc;
        break;
    }
    case PathologyPropertyInfo::Comb_RASMutation:
    {
        QList<PathologyPropertyInfo::Property> rases;
        rases << PathologyPropertyInfo::Mut_KRAS_2;
        rases << PathologyPropertyInfo::Mut_KRAS_3;
        rases << PathologyPropertyInfo::Mut_KRAS_4;
        rases << PathologyPropertyInfo::Mut_NRAS_2_4;
        listOfMutationsResult(disease, rases);
        break;
    }
    case PathologyPropertyInfo::Comb_KRASMutation:
    {
        QList<PathologyPropertyInfo::Property> rases;
        rases << PathologyPropertyInfo::Mut_KRAS_2;
        rases << PathologyPropertyInfo::Mut_KRAS_3;
        rases << PathologyPropertyInfo::Mut_KRAS_4;
        listOfMutationsResult(disease, rases);
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
    return id;
}

QString CombinedValue::toDisplayString() const
{
    // "+" or "-"
    ValueTypeCategoryInfo ownType(info);
    QString str = ownType.toShortDisplayString(result());

    switch (info.property)
    {
    case PathologyPropertyInfo::Comb_HER2:
    {
        // "1+" etc. org "+"/"-"
        ValueTypeCategoryInfo detInfoType(PathologyPropertyInfo::info(determiningProperty.property));
        QString propStr = detInfoType.toShortDisplayString(determiningProperty);

        if (determiningProperty.property == PathologyPropertyInfo(PathologyPropertyInfo::IHC_HER2_DAKO).id)
        {
            str += " (IHC " + propStr + ")";
        }
        else
        {
            float ratio = textCENRatioToFloat(determiningProperty.detail);
            str += " (FISH ";
            str += resultValue.toBool() ? "pos" : "neg";
            str += ", ";
            str += QString::number(ratio, 'f', 2);
            str += ")";
        }
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
    case PathologyPropertyInfo::Comb_cMetIHC3plusScore:
    {
        if (resultValue.toBool())
        {
            ValueTypeCategoryInfo ihcType(PathologyPropertyInfo::IHC_cMET);
            int strongCells = ihcType.toMedicalValue(determiningProperty).value<HScore>().percentageStrong();
            str += QString(" (%1%%)").arg(strongCells);
        }
    }
    default:
        return str;
    }
}
