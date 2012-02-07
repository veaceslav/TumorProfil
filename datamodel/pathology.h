/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 2012-01-22
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

#ifndef PATHOLOGY_H
#define PATHOLOGY_H

// Qt includes

#include <QList>
#include <QSharedPointer>

// Local includes

#include "property.h"

class Pathology
{
public:

    enum Entity
    {
        UnknownEntity = 0,
        PulmonaryAdeno = 1,
        PulmonarySquamous,
        PulmonaryLargeCell,
        PulmonaryAdenosquamous,
        PulmonaryBronchoalveloar,
        ColorectalAdeno,

        FirstEntity = PulmonaryAdeno,
        LastEntity  = ColorectalAdeno
    };

    enum SampleOrigin
    {
        UnknownOrigin,
        Primary,
        Metastasis
    };

    typedef QSharedPointer<Pathology> Ptr;

    Pathology();

    Entity          entity;
    SampleOrigin    sampleOrigin;
    QList<Property> properties;

    int             id;
};

#endif // PATHOLOGY_H
