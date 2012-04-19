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
                       const Patient::Ptr& p, int role, int field);

    Patient::Ptr p;
    bool         hasPathology;
    int          field;
    int          role;
    QList<PathologyPropertyInfo>  infos;

protected:

    QVariant entityNameDatum();
    QVariant fieldDatum();
    QVariant fieldHeader();
    QVariant otherMutationsDatum();
    QVariant otherMutationsHeader();

    enum CompletenessField
    {
        IHCCompleteness,
        MutationCompleteness,
    };
    QVariant completenessDatum(CompletenessField value);
    QVariant completenessHeader(CompletenessField value);

};


#endif // MODELDATAGENERATOR_H
