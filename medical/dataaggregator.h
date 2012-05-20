/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 15.05.2012
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

#ifndef DATAAGGREGATOR_H
#define DATAAGGREGATOR_H

#include <QMetaType>

#include "pathologypropertyinfo.h"
#include "property.h"

class AggregatedDatumInfo
{
public:

    enum Field
    {
        InvalidField,
        Count,
        Positive,
        Negative,
        IHC_1,
        IHC_2,
        IHC_3
    };

    enum ValueType
    {
        InvalidValue,
        AbsoluteValue,
        PercentageValue,
        ConfidenceUpper,
        ConfidenceLower
    };

    AggregatedDatumInfo();
    AggregatedDatumInfo(Field field, ValueType valueType);

    bool isValid() const;

    /** Takes a possible value from ValueTypeCategoryInfo's possibleValues */
    static QList<AggregatedDatumInfo> fieldsFromCategory(const ValueTypeCategoryInfo& info);
    static QList<AggregatedDatumInfo> possibleValueTypes(Field field);

    bool operator==(const AggregatedDatumInfo& other) const;
    bool operator<(const AggregatedDatumInfo& other) const;
    QString label() const;

    Field     field;
    ValueType valueType;
};

inline uint qHash(AggregatedDatumInfo key) { return (key.field << 16) + key.valueType; }
QDebug operator<<(QDebug dbg, const AggregatedDatumInfo &a);

class DataAggregator
{
public:

    DataAggregator(const PathologyPropertyInfo& field);

    DataAggregator& operator<<(const Property& prop);
    QMap<AggregatedDatumInfo, QVariant> values() const;
    bool isCountedAs(const Property& prop, const AggregatedDatumInfo& info) const;

protected:

    QVariant aggregate(const AggregatedDatumInfo& datumInfo,
                       const QVariantList& values) const;

    PathologyPropertyInfo field;
    PropertyList          properties;
};

Q_DECLARE_METATYPE(AggregatedDatumInfo)

#endif // DATAAGGREGATOR_H
