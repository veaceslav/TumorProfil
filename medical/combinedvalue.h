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
#include "patient.h"
#include "property.h"

class CombinedValue
{
public:
    CombinedValue(const PathologyPropertyInfo& info);

    Property combine(Patient::Ptr p);

protected:

    PathologyPropertyInfo info;
};

#endif // COMBINEDVALUE_H
