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

#ifndef ACTIONABLERESULTCHECKER_H
#define ACTIONABLERESULTCHECKER_H

#include <QFlags>

#include <pathologypropertyinfo.h>

#include "patient.h"

class DataAggregator;

class ActionableResultChecker
{
public:

    enum Flag
    {
        NoFlags     = 0,
        IncludeRAS = 1 << 0,
        IncludeReceptorStatus = 1 << 1,
        IncludePTEN = 1 << 2,
        IncludeP53
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    ActionableResultChecker(const Patient::Ptr& p, Flags flags = NoFlags);

    QList<PathologyPropertyInfo> actionableResults();
    QVariant hasResults(const QList<PathologyPropertyInfo>& combination);

    // Returns a complex data structure of aggregated actionable results of the given patients.
    // For all seen combinations of actionable properties (as keys in the map), an aggregator is created
    // which counts the patients presenting with this combination.
    // Note that you must delete the DataAggregator objects after usage.
    static QMap< QList<PathologyPropertyInfo>, DataAggregator* > actionableCombinations(const QList<Patient::Ptr>& patients, Flags flags);

protected:

    void fillFields(const Disease& disease);
    void checkFields(const Disease& disease,
                     QList<PathologyPropertyInfo> &actionableFields,
                     QList<PathologyPropertyInfo> fieldsToCheck,
                     bool valueToCheck);
    QList<Property> fields(const PathologyPropertyInfo& info, const Disease& disease);


    Patient::Ptr const p;
    Flags flags;
    QList<PathologyPropertyInfo> positiveFields;
    QList<PathologyPropertyInfo> negativeFields;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ActionableResultChecker::Flags)

#endif // ACTIONABLERESULTCHECKER_H
