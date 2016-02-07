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

#include <QDate>
#include <QList>
#include <QSharedPointer>
#include <QStringList>

// Local includes

#include "property.h"

class Pathology
{
public:

    // DATABASE CONSTANTS
    enum Entity
    {
        UnknownEntity = 0,
        PulmonaryAdeno = 1,
        PulmonarySquamous        = 2,
        PulmonaryLargeCell       = 3,
        PulmonaryAdenosquamous   = 4,
        PulmonaryBronchoalveloar = 5,
        ColorectalAdeno          = 6,
        Cholangiocarcinoma       = 7,
        RenalCell                = 8,
        Esophageal               = 9,
        EsophagogastrealJunction = 10,
        Gastric                  = 11,
        PulmonaryOtherCarcinoma  = 12,
        Breast                   = 13,
        TransitionalCell         = 14,
        Thyroid                  = 15,
        Melanoma                 = 16,
        // Only add to the end; database constants!!

        FirstEntity = PulmonaryAdeno,
        LastEntity  = Melanoma
    };

    // DATABASE CONSTANTS
    enum SampleOrigin
    {
        UnknownOrigin,
        Primary,
        Metastasis,
        LocalLymphNode
    };

    typedef QSharedPointer<Pathology> Ptr;

    Pathology();
    bool operator==(const Pathology& other) const;

    Entity          entity;
    SampleOrigin    sampleOrigin;
    QString         context;
    QDate           date;
    PropertyList    properties;
    QStringList     reports;

    int             id;
};

#endif // PATHOLOGY_H
