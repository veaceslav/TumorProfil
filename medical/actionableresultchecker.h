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

#include <pathologypropertyinfo.h>

#include "patient.h"

class ActionableResultChecker
{
public:
    ActionableResultChecker(const Patient::Ptr& p);

    QList<PathologyPropertyInfo> actionableResults();

protected:

    Patient::Ptr const p;
};

#endif // ACTIONABLERESULTCHECKER_H
