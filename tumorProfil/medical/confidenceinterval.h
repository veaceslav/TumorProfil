/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 20.05.2012
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

#ifndef CONFIDENCEINTERVAL_H
#define CONFIDENCEINTERVAL_H

#include <QPair>

class ConfidenceInterval
{
public:
    ConfidenceInterval();
    void setConfidence(double ci);
    void setEvents(unsigned int events);
    void setObservations(unsigned int observations);

    QPair<double, double> binomial();

protected:

    typedef long double Double;

    Double binomP(Double N, Double p, Double x1, Double x2);

    double         m_ci;
    unsigned int   m_events;
    unsigned int   m_observations;
};

#endif // CONFIDENCEINTERVAL_H
