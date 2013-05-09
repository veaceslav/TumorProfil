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

#include "ihcscore.h"

// Boost includes

#include <boost/icl/interval_map.hpp>
#include <iostream>

// Qt includes

#include <QDebug>
#include <QRegExp>
#include <QStringList>

IHCScore::IHCScore()
    : colorIntensity(InvalidIntensity)
{
}

IHCScore::IHCScore(Intensity intensity, const QString& detail)
    : colorIntensity(intensity)
{
    parseCells(detail);
}

IHCScore::IHCScore(const QVariant& value, const QString& detail)
    : colorIntensity((Intensity)value.toInt())
{
    parseCells(detail);
}

bool IHCScore::isNull() const
{
    return colorIntensity == InvalidIntensity;
}

bool IHCScore::isValid() const
{
    return colorIntensity != InvalidIntensity &&
            (colorIntensity == NoIntensity // in which case, interval may be empty
               ||
            (!boost::icl::is_empty(percentagePositiveCells)
            && percentagePositiveCells.lower() >= 0 && percentagePositiveCells.lower() <= 100
            && percentagePositiveCells.upper() >= 0 && percentagePositiveCells.upper() <= 100
            && percentagePositiveCells.lower() <= percentagePositiveCells.upper()
             )
            )
             ;
}


static int scoreFromInterval(boost::icl::discrete_interval<int> percentagePositiveCells)
{
    boost::icl::interval_map<int, int> scores;
    scores += std::make_pair(boost::icl::interval<int>::closed(0,0), 0);
    scores += std::make_pair(boost::icl::interval<int>::closed(1,10), 1);
    scores += std::make_pair(boost::icl::interval<int>::closed(11,50), 2);
    scores += std::make_pair(boost::icl::interval<int>::closed(51,100), 3);
    //std::cout << scores << std::endl;

    boost::icl::interval_map<int, int> value;
    value += std::make_pair(percentagePositiveCells, 1);
    //std::cout << value << std::endl;

    boost::icl::interval_map<int, int> result = scores & value;
    //std::cout << result << std::endl;
    QMap<int, int> scoredIntervals;
    boost::icl::interval_map<int, int>::iterator it;
    for (it = result.begin(); it != result.end(); ++it)
    {
        scoredIntervals[it->first.upper() - it->first.lower()] = it->second;
    }
    //qDebug() << scoredIntervals;
    if (!scoredIntervals.isEmpty())
    {
        return (scoredIntervals.end()-1).value() - 1;
    }
    return 0;
}

QVariant IHCScore::score() const
{
    if (isNull())
    {
        return QVariant();
    }
    if (isValid())
    {
        return colorIntensity * scoreFromInterval(percentagePositiveCells);
    }
    // Assume value is missing / invalid, yet we have a valid intensity => boolean value
    return colorIntensity != NoIntensity;
}

bool IHCScore::isPositive(PathologyPropertyInfo::Property field) const
{
    QVariant s = score();
    switch (field)
    {
    // TODO: specify "positive" depending on field
    case PathologyPropertyInfo::IHC_HER2:
        // Positivity of "scored" Her2 is undefined, only defined for DAKO!
        if (s.type() == QVariant::Int)
            return s.toInt() >= 4;
        else
            return false;
    case PathologyPropertyInfo::IHC_PTEN:
        if (s.type() == QVariant::Int)
            return s.toInt() >= 2;
        else
            return s.toBool();
    default:
        if (s.type() == QVariant::Int)
            return s.toInt() > 1;
        else
            return s.toBool();
    }
}

void IHCScore::parseCells(const QString &detail)
{
    if (detail.isEmpty())
    {
        return;
    }
    QRegExp regexp("\\s*(\\d*)\\s*([<>=-]*)\\s*(\\d*)\\s*%?");
    if (!regexp.exactMatch(detail))
    {
        qDebug() << "Cannot parse cell count" << detail;
        return;
    }

    QString leftLimit = regexp.cap(1);
    QString op = regexp.cap(2);
    QString rightLimit = regexp.cap(3);

    if (leftLimit.isEmpty() && rightLimit.isEmpty())
    {
        qDebug() << "Invalid cell count" << detail;
        return;
    }
    if (!leftLimit.isEmpty() && !rightLimit.isEmpty())
    {
        percentagePositiveCells = boost::icl::interval<int>::closed(leftLimit.toInt(), rightLimit.toInt());
    }
    else if (!leftLimit.isEmpty() && rightLimit.isEmpty())
    {
        percentagePositiveCells = boost::icl::interval<int>::closed(leftLimit.toInt(), leftLimit.toInt());
    }
    else if (!leftLimit.isEmpty() && op == "-")
    {
        percentagePositiveCells = boost::icl::interval<int>::right_open(leftLimit.toInt(), rightLimit.toInt());
    }
    else if (!rightLimit.isEmpty() && op.contains("<"))
    {
        if (op.contains("="))
        {
            percentagePositiveCells = boost::icl::interval<int>::closed(0, rightLimit.toInt());
        }
        else
        {
            percentagePositiveCells = boost::icl::interval<int>::right_open(0, rightLimit.toInt());
        }
    }
    else if (!rightLimit.isEmpty() && op.contains(">"))
    {
        int left = rightLimit.toInt();
        int missingRightLimit = (left<10 ? 10 : (left < 50 ? 50 : 100));
        if (op.contains("="))
        {
            percentagePositiveCells = boost::icl::interval<int>::closed(left, missingRightLimit);
        }
        else
        {
            percentagePositiveCells = boost::icl::interval<int>::right_open(left, missingRightLimit);
        }
    }
    else
    {
        qDebug() << "Invalid cell count" << detail;
        return;
    }
}

static int threeToOne(int strong, int medium, int weak)
{
    int i = 0;
    i |= ( ((uchar)strong) << 16 );
    i |= ( ((uchar)medium) <<  8 );
    i |= ( ((uchar)weak)       );
    return i;
}

HScore::HScore()
    : binaryScore(-1)
{
}

HScore::HScore(int strong, int medium, int weak)
    : binaryScore(threeToOne(strong, medium, weak))
{
}

HScore::HScore(int binaryScore)
    : binaryScore(binaryScore)
{
}

HScore::HScore(QVariant binaryScore)
    : binaryScore(binaryScore.toInt())
{
}

bool HScore::isNull() const
{
    return binaryScore == -1;
}

bool HScore::isValid() const
{
    int sc = score().toInt();
    return sc >= 0 && sc <= 300 && percentageStrong()+percentageMedium()+percentageWeak()<=100;
}

QVariant HScore::score() const
{
    return percentageStrong() * 3 + percentageMedium() * 2 + percentageWeak();
}

QVector<int> HScore::percentages() const
{
    QVector<int> ret;
    ret << percentageStrong();
    ret << percentageMedium();
    ret << percentageWeak();
    ret << percentageNone();
    return ret;
}

int HScore::percentageStrong() const
{
    return (binaryScore & (0xFF << 16)) >> 16;
}

int HScore::percentageMedium() const
{
    return (binaryScore & (0xFF << 8)) >> 8;
}

int HScore::percentageWeak() const
{
    return binaryScore & 0xFF;
}

int HScore::percentageNone() const
{
    return qMax(0, 100 - percentageStrong() - percentageMedium() - percentageWeak());
}
