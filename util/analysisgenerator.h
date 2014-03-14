/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 10.03.2014
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

#ifndef ANALYSISGENERATOR_H
#define ANALYSISGENERATOR_H

#include "csvfile.h"
#include "pathologypropertyinfo.h"

class Disease;


class AnalysisGenerator
{
public:
    AnalysisGenerator();

    void her2();

protected:

    QVariant writePathologyProperty(const Disease& disease, PathologyPropertyInfo::Property id);

    CSVFile m_file;
};

#endif // ANALYSISGENERATOR_H
