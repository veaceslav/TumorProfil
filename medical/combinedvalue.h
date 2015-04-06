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

#ifndef COMBINEDVALUE_H
#define COMBINEDVALUE_H

#include "pathologypropertyinfo.h"
#include "disease.h"
#include "property.h"

class CombinedValue
{
public:
    CombinedValue(const PathologyPropertyInfo& info);

    void combine(const Disease& disease);
    Property result() const;
    QString  toDisplayString() const;
    QVariant toValue() const;
    QVariant toCombinedVariant() const;
    bool isValid() const;

    // contains some extra code beyond toMedicalValue
    QVariant fishResult(const Disease& disease, const Property& prop);
    // additionally retrieves the property
    QVariant fishResult(const Disease& disease);

protected:

    QVariant listOfMutationsResult(const Disease& disease, const QList<PathologyPropertyInfo::Property>& propIds);

    QVariant resultValue;
    Property determiningProperty;

    const PathologyPropertyInfo info;
};

#endif // COMBINEDVALUE_H
