/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 17.07.2012
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

#ifndef IHCSCORE_H
#define IHCSCORE_H

// Boost includes

#include <boost/icl/discrete_interval.hpp>
#include <boost/icl/interval_set.hpp>

// Qt includes

#include <QString>
#include <QVariant>

class IHCScore
{
public:

    enum Intensity
    {
        InvalidIntensity = -1,
        NoIntensity      = 0,
        WeakIntensity    = 1,
        MediumIntensity  = 2,
        StrongIntensity  = 3
    };

    IHCScore();
    IHCScore(Intensity value, const QString& detail);
    IHCScore(const QVariant& value, const QString& detail);

    // A null score has the InvalidIntensity
    bool isNull() const;
    // Has a valid intensity and a non-empty interval within 0 and 100
    bool isValid() const;

    boost::icl::discrete_interval<int> percentagePositiveCells;
    Intensity colorIntensity;

    QVariant score() const;

protected:

    void parseCells(const QString& detail);
};


#endif // IHCSCORE_H
