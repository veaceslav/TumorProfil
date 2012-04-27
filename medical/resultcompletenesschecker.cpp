/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 18.04.2012
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

#include "resultcompletenesschecker.h"

ResultCompletenessChecker::ResultCompletenessChecker(const Patient::Ptr& p)
    : p(p)
{
}

ResultCompletenessChecker::CompletenessResult ResultCompletenessChecker::isIHCComplete(QList<PathologyPropertyInfo>* missingProperties)
{
    const Disease& disease = p->firstDisease();
    QList<PathologyPropertyInfo> requiredFields;
    switch (disease.entity())
    {
    case Pathology::PulmonaryAdeno:
    case Pathology::PulmonaryBronchoalveloar:
    case Pathology::PulmonaryAdenosquamous:
        requiredFields << PathologyPropertyInfo::IHC_pAKT
                       << PathologyPropertyInfo::IHC_ALK
                       << PathologyPropertyInfo::IHC_pERK
                       << PathologyPropertyInfo::IHC_HER2
                       << PathologyPropertyInfo::IHC_PTEN;
        break;
    case Pathology::PulmonarySquamous:
        requiredFields << PathologyPropertyInfo::IHC_pAKT
                       << PathologyPropertyInfo::IHC_pERK
                       << PathologyPropertyInfo::IHC_PTEN;
        break;
    case Pathology::ColorectalAdeno:
        requiredFields << PathologyPropertyInfo::IHC_pAKT
                       << PathologyPropertyInfo::IHC_pP70S6K
                       << PathologyPropertyInfo::IHC_pERK
                       << PathologyPropertyInfo::IHC_PTEN;
        break;
    default:
        return Undefined;
    }

    bool complete = true;
    QList<PathologyPropertyInfo> missing;
    foreach (const PathologyPropertyInfo& field, requiredFields)
    {
        if (!disease.hasPathologyProperty(field.id))
        {
            complete = false;
            missing << field;
        }
    }
    if (missingProperties)
    {
        *missingProperties = missing;
    }

    if (complete)
    {
        return Complete;
    }
    else if (requiredFields.size() == missing.size())
    {
        return Absent;
    }
    else
    {
        return Incomplete;
    }
}

ResultCompletenessChecker::CompletenessResult completenessCascade(const Disease& disease,
    const QList<PathologyPropertyInfo> requiredFields, QList<PathologyPropertyInfo>* missingProperties)
{
    bool hasAny = false;
    foreach (const PathologyPropertyInfo& field, requiredFields)
    {
        Property prop = disease.pathologyProperty(field.id);
        if (prop.isValid())
        {
            hasAny = true;
            // in case of a mutation, the algorithm stops
            if (ValueTypeCategoryInfo(field).toValue(prop.value).toBool())
            {
                break;
            }
            // else check next
        }
        else
        {
            if (missingProperties)
            {
                *missingProperties << field;
            }
            return hasAny ? ResultCompletenessChecker::Absent : ResultCompletenessChecker::Incomplete;
        }
    }
    return ResultCompletenessChecker::Complete;
}

QVariant checkCompleteness(const Disease& disease, PathologyPropertyInfo::Property property,
                           bool& complete, QList<PathologyPropertyInfo>* missingProperties)
{
    PathologyPropertyInfo field(property);
    Property prop = disease.pathologyProperty(field.id);
    if (!prop.isValid())
    {
        complete = false;
        if (missingProperties)
        {
            *missingProperties << field;
        }
        return QVariant();
    }
    return ValueTypeCategoryInfo(field).toValue(prop.value);
}

ResultCompletenessChecker::CompletenessResult ResultCompletenessChecker::isMutComplete(QList<PathologyPropertyInfo>* missingProperties)
{
    const Disease& disease = p->firstDisease();
    QList<PathologyPropertyInfo> requiredFields;
    switch (disease.entity())
    {
    case Pathology::PulmonaryAdeno:
    case Pathology::PulmonaryBronchoalveloar:
    case Pathology::PulmonaryAdenosquamous:
        requiredFields << PathologyPropertyInfo::Mut_KRAS_2
                       << PathologyPropertyInfo::Mut_EGFR_19_21
                       << PathologyPropertyInfo::Mut_PIK3CA_10_21
                       << PathologyPropertyInfo::Mut_BRAF_15;
        return completenessCascade(disease, requiredFields, missingProperties);
        break;
    case Pathology::PulmonarySquamous:
        return OnlyOptional;
        break;
    case Pathology::ColorectalAdeno:
    {
        bool complete = true;
        QVariant kras = checkCompleteness(disease, PathologyPropertyInfo::Mut_KRAS_2, complete, missingProperties);
        checkCompleteness(disease, PathologyPropertyInfo::Mut_PIK3CA_10_21, complete, missingProperties);
        // kras Wildtyp?
        if (kras.isValid() && !kras.toBool())
        {
            checkCompleteness(disease, PathologyPropertyInfo::Mut_BRAF_15, complete, missingProperties);
            checkCompleteness(disease, PathologyPropertyInfo::Mut_KRAS_3, complete, missingProperties);
        }
        return complete ? Complete : Incomplete;
    }
    default:
        break;
    }
    return Undefined;
}