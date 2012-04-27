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

#ifndef RESULTCOMPLETENESSCHECKER_H
#define RESULTCOMPLETENESSCHECKER_H

#include <patient.h>
#include <pathologypropertyinfo.h>

class ResultCompletenessChecker
{
public:
    ResultCompletenessChecker(const Patient::Ptr& p);

    enum CompletenessResult
    {
        Undefined,
        Absent,
        Incomplete,
        Complete,
        OnlyOptional
    };

    CompletenessResult isIHCComplete(QList<PathologyPropertyInfo>* missingProperties);
    CompletenessResult isMutComplete(QList<PathologyPropertyInfo>* missingProperties);

protected:

    Patient::Ptr const p;
};

#endif // RESULTCOMPLETENESSCHECKER_H
