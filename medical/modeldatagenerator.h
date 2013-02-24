/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 12.04.2012
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

#ifndef MODELDATAGENERATOR_H
#define MODELDATAGENERATOR_H

// Local includes

#include <patient.h>
#include <pathologypropertyinfo.h>

class ModelDataGenerator
{
public:

    ModelDataGenerator(const QList<PathologyPropertyInfo>&  infos,
                       const Patient::Ptr& p, int role, int field,
                       PathologyContextInfo::Context primaryContext = PathologyContextInfo::Tumorprofil);

    Patient::Ptr p;
    bool         hasPathology;
    int          field;
    int          role;
    QList<PathologyPropertyInfo>  infos;
    PathologyContextInfo::Context primaryContext;

protected:

    QVariant entityNameDatum();
    enum AdditionalInfoColumns
    {
        GenderColumn,
        AgeColumn,
        InitialDiagnosisColumn,
        DiseaseAgeColumn,
        ResultDateColumn,
        ResultLocationColumn,
        DiseaseAgeAtResultColumn,
        PatientAdditionalInfoColumnCount
    };
    QVariant additionalInfoDatum();
    QVariant additionalInfoHeader();
    QVariant fieldDatum();
    QVariant fieldHeader();
    QVariant otherMutationsDatum();
    QVariant otherMutationsHeader();

    enum CompletenessField
    {
        IHCCompleteness,
        MutationCompleteness,
        FISHCompleteness
    };
    QVariant completenessDatum(CompletenessField value);
    QVariant completenessHeader(CompletenessField value);

    enum ActionableResultsField
    {
        ActionableResultFieldNames,
        ActionableResultNumber
    };

    QVariant actionableResultsDatum(ActionableResultsField value);
    QVariant actionableResultsHeader(ActionableResultsField value);

    QVariant returnDate(const QDate& date);

};


#endif // MODELDATAGENERATOR_H
