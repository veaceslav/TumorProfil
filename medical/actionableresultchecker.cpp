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
#include "disease.h"
#include "pathology.h"

ActionableResultChecker::ActionableResultChecker(const Patient::Ptr& p)
    : p(p)
{
}

static void checkFields(const Disease& disease,
                        QList<PathologyPropertyInfo> &actionableFields,
                        QList<PathologyPropertyInfo> fieldsToCheck,
                        bool valueToCheck)
{
    foreach (const PathologyPropertyInfo& field, fieldsToCheck)
    {
        foreach (const Property& prop, disease.pathologyProperties(field.id))
        {
            ValueTypeCategoryInfo valueType(field);
            if (valueType.toValue(prop.value).toBool() == valueToCheck)
            {
                actionableFields << field;
                break;
            }
        }
    }
}

QList<PathologyPropertyInfo> ActionableResultChecker::actionableResults()
{
    const Disease& disease = p->firstDisease();
    QList<PathologyPropertyInfo> positiveFields, negativeFields;
    switch (disease.entity())
    {
    case Pathology::PulmonarySquamous:
        positiveFields << PathologyPropertyInfo::Mut_DDR2
                       << PathologyPropertyInfo::Fish_FGFR1;
    case Pathology::PulmonaryAdeno:
    case Pathology::PulmonaryBronchoalveloar:
    case Pathology::PulmonaryAdenosquamous:
    case Pathology::ColorectalAdeno:
        positiveFields << PathologyPropertyInfo::Mut_EGFR_18_20
                       << PathologyPropertyInfo::Mut_EGFR_19_21
                       << PathologyPropertyInfo::Mut_BRAF_11
                       << PathologyPropertyInfo::Mut_BRAF_15
                       /*<< PathologyPropertyInfo::Mut_KRAS_2
                       << PathologyPropertyInfo::Mut_KRAS_3*/
                       << PathologyPropertyInfo::Fish_ALK;
    default:
        positiveFields << PathologyPropertyInfo::Mut_PIK3CA_10_21
                       << PathologyPropertyInfo::Fish_HER2
                       << PathologyPropertyInfo::Fish_PIK3CA;
        negativeFields << PathologyPropertyInfo::IHC_PTEN;
        break;
    }

    QList<PathologyPropertyInfo> actionableFields;
    checkFields(disease, actionableFields, positiveFields, true);
    checkFields(disease, actionableFields, negativeFields, false);
    return actionableFields;
}
