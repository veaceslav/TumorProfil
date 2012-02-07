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

#ifndef DISEASE_H
#define DISEASE_H

// Qt includes

#include <QList>
#include <QDate>
#include <QSharedPointer>

// Local includes

#include "pathology.h"
#include "tnm.h"

class Disease
{
public:

    Disease();

    typedef QSharedPointer<Disease> Ptr;

public:

    QDate initialDiagnosis;
    TNM   initialTNM;
    QList<Pathology> pathologies;

    int   id;

    bool hasPathology() const { return !pathologies.isEmpty(); }
};

#endif // DISEASE_H
