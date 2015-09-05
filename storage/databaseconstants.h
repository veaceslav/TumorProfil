/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 28.02.2013
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

#ifndef DATABASECONSTANTS_H
#define DATABASECONSTANTS_H

#include <QLatin1String>

class PatientPropertyName
{
public:

    static QLatin1String trialParticipation();
};


class DiseasePropertyName
{
public:

    static QLatin1String diseaseHistory();
};

class PathologyPropertyName
{
public:

    // Note: Most pathology property names are defined in pathologypropertyinfo.cpp
    // Here come only those with a "special" meaning
    static QLatin1String pathologyReportId();
};

#endif // DATABASECONSTANTS_H
