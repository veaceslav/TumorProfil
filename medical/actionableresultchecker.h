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

class ActionableResultChecker
{
public:

    enum Flag
    {
        NoFlags     = 0,
        IncludeKRAS = 1 << 0,
        IncludeReceptorStatus = 1 << 1
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    ActionableResultChecker(const Patient::Ptr& p, Flags flags = NoFlags);

    QList<PathologyPropertyInfo> actionableResults();
    QVariant hasResults(const QList<PathologyPropertyInfo>& combination);

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

#endif // ACTIONABLERESULTCHECKER_H
