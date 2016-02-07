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

#include "confidenceinterval.h"

// Qt includes

#include <QDebug>

ConfidenceInterval::ConfidenceInterval()
    : m_ci(0.95), m_events(0), m_observations(0)
{
}

void ConfidenceInterval::setConfidence(double ci)
{
    m_ci = ci;
}

void ConfidenceInterval::setEvents(unsigned int events)
{
    m_events = events;
}

void ConfidenceInterval::setObservations(unsigned int observations)
{
    m_observations = observations;
}

QPair<double, double> ConfidenceInterval::binomial()
{
    QPair<double, double> result;
    if (m_events > m_observations)
    {
        qDebug() << "Events > Observations";
        return result;
    }
    if (m_observations < 1)
    {
        qDebug() << "Observations < 1";
        return result;
    }
    if (m_ci <0 || m_ci > 1)
    {
        qDebug() << "Invalid confidence" << m_ci;
        return result;
    }

    Double p = (1-m_ci) / 2; // one half of p for each side

    if (m_events == 0)
    {
        result.first = 0;
    }
    else
    {
        Double vp = Double(m_events) / Double(m_observations);
        Double v = vp / 2;
        Double vsl = 0, vsh = vp;
        while ((vsh - vsl) > 1e-10)
        {
            //qDebug() << "vp" << vp << "v" << v << "vsl" << vsl << "vsh" << vsh << "binom" << binomP(m_observations, v, m_events, m_observations);
            if (binomP(m_observations, v, m_events, m_observations) > p)
            {
                vsh = v;
                v = (vsl + v) / 2;
            }
            else
            {
                vsl = v;
                v = (v + vsh) / 2;
            }
        }
        result.first = v;
    }

    if (m_events == m_observations)
    {
        result.second = 1;
    }
    else
    {
        Double vp = Double(m_events) / Double(m_observations);
        Double v  = (1 + vp) / 2;
        Double vsl = vp, vsh = 1;
        while ((vsh - vsl) > 1e-10)
        {
            if (binomP(m_observations, v, 0, m_events) < p)
            {
                vsh = v;
                v = (vsl + v) / 2;
            }
            else
            {
                vsl = v;
                v = (v + vsh) / 2;
            }
        }
        result.second = v;
    }
    return result;
}

ConfidenceInterval::Double ConfidenceInterval::binomP(Double N, Double p, Double x1, Double x2)
{
    if (N < 1)
    {
        qDebug() << "N < 1";
        return 0;
    }
    if (p < 0 || p > 1)
    {
        qDebug() << "Invalid p" << double(p);
        return 0;
    }
    if (x1 < 0 || x1 > N || x2 < 0 || x2 > N || x1 >= x2)
    {
        //qDebug() << "Invalid parameters" << double(x1) << double(x2) << double(N);
    }

    if (p == 1)
    {
        if (x2 == N)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    Double q = p / (1-p);
    Double k=0, v=1, S=0, Tot=0;
    while (k <= N)
    {
        //qDebug() << "q" << q << "k" << k << "v" << v << "S" << S << "Tot" << Tot;
        Tot = Tot + v;
        if (k >= x1 && k <= x2)
        {
            S += v;
        }
        if (Tot > 1e+30)
        {
            S = S / 1E+30;
            Tot = Tot / 1E+30;
            v = v / 1E+30;
        }
        k += 1;
        v = v * q * (N + 1 - k) / k;
    }
    return S / Tot;
}
