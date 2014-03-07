/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 15.05.2012
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

#include "actionableresultchecker.h"
#include "combinedvalue.h"
#include "dataaggregator.h"
#include "disease.h"
#include "pathology.h"

ActionableResultChecker::ActionableResultChecker(const Patient::Ptr& p, Flags flags)
    : p(p), flags(flags)
{
    if (!p->hasDisease())
    {
        return;
    }
    fillFields(p->firstDisease());
}

void ActionableResultChecker::checkFields(const Disease& disease,
                                          QList<PathologyPropertyInfo> &actionableFields,
                                          QList<PathologyPropertyInfo> fieldsToCheck,
                                          bool valueToCheck)
{
    foreach (const PathologyPropertyInfo& field, fieldsToCheck)
    {
        foreach (const Property& prop, fields(field, disease))
        {
            ValueTypeCategoryInfo valueType(field);
            bool isPositive = DataAggregator::isPositive(field, valueType.toMedicalValue(prop));
            if (isPositive == valueToCheck)
            {
                actionableFields << field;
                break;
            }
        }
    }
}

QList<Property> ActionableResultChecker::fields(const PathologyPropertyInfo& info,
                                                             const Disease& disease)
{
    if (info.isCombined())
    {
        CombinedValue combinedValue(info);
        combinedValue.combine(disease);
        Property prop = combinedValue.result();
        if (prop.isNull())
        {
            return QList<Property>();
        }
        return QList<Property>() << prop;
    }
    return disease.pathologyProperties(info.id);
}

void ActionableResultChecker::fillFields(const Disease &disease)
{
    positiveFields << PathologyPropertyInfo::Mut_PIK3CA_10_21
                   << PathologyPropertyInfo::Mut_PTEN
                   << PathologyPropertyInfo::Fish_HER2
                   << PathologyPropertyInfo::Fish_PIK3CA
                   << PathologyPropertyInfo::Comb_cMetActivation
                   << PathologyPropertyInfo::Fish_ROS1
                   << PathologyPropertyInfo::Fish_FGFR1;
    negativeFields << PathologyPropertyInfo::IHC_PTEN;

    switch (disease.entity())
    {
    case Pathology::PulmonarySquamous:
        positiveFields << PathologyPropertyInfo::Mut_DDR2
                       << PathologyPropertyInfo::Fish_FGFR1;
    case Pathology::PulmonaryAdeno:
    case Pathology::PulmonaryBronchoalveloar:
    case Pathology::PulmonaryAdenosquamous:
        positiveFields << PathologyPropertyInfo::Comb_HER2;
    case Pathology::ColorectalAdeno:
        positiveFields << PathologyPropertyInfo::Mut_EGFR_18_20
                       << PathologyPropertyInfo::Mut_EGFR_19_21
                       << PathologyPropertyInfo::Mut_BRAF_11
                       << PathologyPropertyInfo::Mut_BRAF_15
                       << PathologyPropertyInfo::Fish_ALK;
        if (flags & IncludeKRAS)
        {
            positiveFields
                    << PathologyPropertyInfo::Mut_KRAS_2
                    << PathologyPropertyInfo::Mut_KRAS_3
                    << PathologyPropertyInfo::Mut_KRAS_4
                    << PathologyPropertyInfo::Mut_NRAS_2_4;
        }
        break;
    case Pathology::Breast:
        if (flags & IncludeReceptorStatus)
        {
            positiveFields << PathologyPropertyInfo::IHC_ER
                           << PathologyPropertyInfo::IHC_PR
                           << PathologyPropertyInfo::Comb_HER2;
        }
    default:
        break;
    }
}

QList<PathologyPropertyInfo> ActionableResultChecker::actionableResults()
{
    QList<PathologyPropertyInfo> actionableFields;
    if (!p->hasDisease())
    {
        return actionableFields;
    }
    const Disease& disease = p->firstDisease();
    checkFields(disease, actionableFields, positiveFields, true);
    checkFields(disease, actionableFields, negativeFields, false);
    qSort(actionableFields);
    return actionableFields;
}

QVariant ActionableResultChecker::hasResults(const QList<PathologyPropertyInfo>& combination)
{
    if (!p->hasDisease())
    {
        return false;
    }
    if (combination.isEmpty())
    {
        // return true if patient has no actionable results
        return actionableResults().isEmpty();
    }

    const Disease& disease = p->firstDisease();
    int hasFields = 0;
    int matches   = 0;
    foreach (const PathologyPropertyInfo& field, combination)
    {
        bool valueToCheck = positiveFields.contains(field);
        bool hasField = false;
        bool match   = false;
        foreach (const Property& prop, fields(field, disease))
        {
            hasField = true;
            ValueTypeCategoryInfo valueType(field);
            bool isPositive = DataAggregator::isPositive(field, valueType.toMedicalValue(prop));
            if (isPositive == valueToCheck)
            {
                match = true;
                break;
            }
        }
        if (hasField)
            hasFields++;
        if (match)
            matches++;
    }
    // None of the fields in the combination were found: Return null
    // Less strict: If we have at least one field, we count for the total
    //if (!hasFields)
    // More strict: Count for total only of all fields are available
    if (hasFields < combination.size())
    {
        return QVariant();
    }
    // If at least one of the fields is found, we return a value
    return matches == combination.size();
}
