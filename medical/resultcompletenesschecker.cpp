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

#include "ihcscore.h"

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
                       << PathologyPropertyInfo::IHC_HER2_DAKO
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
    case Pathology::Breast:
        requiredFields << PathologyPropertyInfo::IHC_pAKT
                       << PathologyPropertyInfo::IHC_pP70S6K
                       << PathologyPropertyInfo::IHC_pERK
                       << PathologyPropertyInfo::IHC_PTEN;
        break;
    default:
        return Undefined;
    }

    bool complete = true;
    QList<PathologyPropertyInfo> missing, partial, returnList;
    foreach (const PathologyPropertyInfo& field, requiredFields)
    {
        ValueTypeCategoryInfo categoryInfo(field);

        bool hasField = disease.hasPathologyProperty(field.id);
        if (!hasField)
        {
            complete = false;
            missing << field;
        }
        // Comment out HERE to switch requiring complete two-dim score
        /*else if (categoryInfo.isTwoDimScored())
        {
            Property prop = disease.pathologyProperty(field.id);
            IHCScore score = categoryInfo.toIHCScore(prop);
            if (!score.isValid())
            {
                complete = false;
                partial << field;
            }
        }*/
    }

    returnList = missing;
    if (!partial.isEmpty())
    {
        // we can only return one list, so join
        if (missing.isEmpty())
        {
            returnList = missing + partial;
            partial.clear();
        }
        else
        {
            returnList = partial;
        }
    }

    if (missingProperties)
    {
        *missingProperties = returnList;
    }

    if (complete)
    {
        return Complete;
    }
    else if (missing.isEmpty() && partial.isEmpty())
    {
        return PartialResult;
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
    return ValueTypeCategoryInfo(field).toMedicalValue(prop);
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
    case Pathology::Breast:
    {
        bool complete = true;
        checkCompleteness(disease, PathologyPropertyInfo::Mut_PIK3CA_10_21, complete, missingProperties);
        return complete ? Complete : Incomplete;
    }
    default:
        break;
    }
    return Undefined;
}

ResultCompletenessChecker::CompletenessResult ResultCompletenessChecker::isFishComplete(QList<PathologyPropertyInfo>* missingProperties)
{
    const Disease& disease = p->firstDisease();
    bool checkHer2 = false;
    bool checkAlk = false;
    switch (disease.entity())
    {
    case Pathology::PulmonaryAdeno:
    case Pathology::PulmonaryBronchoalveloar:
    case Pathology::PulmonaryAdenosquamous:
        checkAlk = true;
    case Pathology::Breast:
        checkHer2 = true;
        break;
    default:
        break;
    }

    if (!checkHer2 && !checkAlk)
    {
        return Undefined;
    }

    bool complete = true, discardValue;
    if (checkHer2)
    {
        QVariant dakoscore = checkCompleteness(disease, PathologyPropertyInfo::IHC_HER2_DAKO, discardValue, 0);
        if (dakoscore.toInt() == 2)
        {
            checkCompleteness(disease, PathologyPropertyInfo::Fish_HER2, complete, missingProperties);
        }
    }
    if (checkAlk)
    {
        QVariant score = checkCompleteness(disease, PathologyPropertyInfo::IHC_ALK, discardValue, 0);
        if (score.toInt() != 0)
        {
            checkCompleteness(disease, PathologyPropertyInfo::Fish_ALK, complete, missingProperties);
        }
    }

    return complete ? Complete : Incomplete;
}
