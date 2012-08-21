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

#include "dataaggregator.h"

#include <QDebug>

#include "confidenceinterval.h"
#include "ihcscore.h"

AggregatedDatumInfo::AggregatedDatumInfo()
    : field(InvalidField), valueType(InvalidValue)
{
}

AggregatedDatumInfo::AggregatedDatumInfo(Field field, ValueType valueType)
    : field(field), valueType(valueType)
{
}

bool AggregatedDatumInfo::isValid() const
{
    return field != InvalidField && valueType != InvalidValue;
}

bool AggregatedDatumInfo::operator<(const AggregatedDatumInfo& other) const
{
    if (other.valueType == valueType)
    {
        return field < other.field;
    }
    return valueType < other.valueType;
}

bool AggregatedDatumInfo::operator==(const AggregatedDatumInfo& other) const
{
    return field == other.field && valueType == other.valueType;
}

QDebug operator<<(QDebug dbg, const AggregatedDatumInfo &a)
{
    dbg << a.label();
    return dbg.space();
}

static QString fieldLabel(AggregatedDatumInfo::Field field)
{
    switch (field)
    {
    case AggregatedDatumInfo::Count:
        return QObject::tr("Anzahl");
    case AggregatedDatumInfo::Positive:
        return QObject::tr("Positiv");
    case AggregatedDatumInfo::Negative:
        return QObject::tr("Negativ");
    case AggregatedDatumInfo::IHC_1:
        return QObject::tr("1+");
    case AggregatedDatumInfo::IHC_2:
        return QObject::tr("2+");
    case AggregatedDatumInfo::IHC_3:
        return QObject::tr("3+");
    case AggregatedDatumInfo::InvalidField:
        break;
    }
    return QString();
}

QString AggregatedDatumInfo::label() const
{
    switch (valueType)
    {
    case AbsoluteValue:
        return fieldLabel(field);
    case PercentageValue:
        return fieldLabel(field) + QObject::tr(" (%)");
    case ConfidenceUpper:
        return fieldLabel(field) + QObject::tr(" ob. CI");
    case ConfidenceLower:
        return fieldLabel(field) + QObject::tr(" un. CI");
    case InvalidValue:
        break;
    }
    return QString();
}

QList<AggregatedDatumInfo> AggregatedDatumInfo::fieldsFromCategory(const ValueTypeCategoryInfo& info)
{
    QList<AggregatedDatumInfo> infos;
    infos << possibleValueTypes(Count);
    infos << possibleValueTypes(Positive);
    infos << possibleValueTypes(Negative);
    /*
    if (info.isScored())
    {
        infos << possibleValueTypes(IHC_1);
        infos << possibleValueTypes(IHC_2);
        infos << possibleValueTypes(IHC_3);
    }
    */
    return infos;
}

QList<AggregatedDatumInfo> AggregatedDatumInfo::possibleValueTypes(Field field)
{
    QList<AggregatedDatumInfo> infos;
    switch (field)
    {
    case InvalidField:
        break;
    case Count:
        infos << AggregatedDatumInfo(field, AbsoluteValue);
        break;
    case Positive:
    case Negative:
    case IHC_1:
    case IHC_2:
    case IHC_3:
        infos << AggregatedDatumInfo(field, AbsoluteValue);
        infos << AggregatedDatumInfo(field, PercentageValue);
        infos << AggregatedDatumInfo(field, ConfidenceUpper);
        infos << AggregatedDatumInfo(field, ConfidenceLower);
    }
    return infos;
}


DataAggregator::DataAggregator(const PathologyPropertyInfo& field)
    : field(field)
{
}

DataAggregator& DataAggregator::operator<<(const Property& prop)
{
    properties << prop;
    return *this;
}

QMap<AggregatedDatumInfo, QVariant> DataAggregator::values() const
{
    QMap<AggregatedDatumInfo, QVariant> result;

    ValueTypeCategoryInfo fieldValueType(field);
    QList<AggregatedDatumInfo> fields = AggregatedDatumInfo::fieldsFromCategory(fieldValueType);
    qSort(fields);
    QVariantList values;
    foreach (const Property& prop, properties)
    {
        if (prop.isValid())
        {
            if (fieldValueType.isTwoDimScored())
            {
                values << QVariant::fromValue(fieldValueType.toIHCScore(prop));
            }
            else
            {
                values << fieldValueType.toValue(prop.value);
            }
        }
    }

    foreach (const AggregatedDatumInfo& info, fields)
    {
        result[info] = aggregate(info, values);
    }
    return result;
}

static bool isPositive(const PathologyPropertyInfo& info,
                       const QVariant& value)
{
    if (value.canConvert<IHCScore>())
    {
        return value.value<IHCScore>().isPositive(info.property);
    }
    else
    {
        switch (info.property)
        {
        case PathologyPropertyInfo::IHC_ALK:
        case PathologyPropertyInfo::IHC_HER2:
            return value.toInt() == 3;
        default:
            return value.toBool();
        }
    }
}

static bool matches(AggregatedDatumInfo::Field field, const PathologyPropertyInfo& info,
                    const QVariant& value)
{
    switch (field)
    {
    case AggregatedDatumInfo::Count:
        return true;
    case AggregatedDatumInfo::Positive:
        return isPositive(info, value);
    case AggregatedDatumInfo::Negative:
        return !isPositive(info, value);
    case AggregatedDatumInfo::IHC_1:
        return (value.toInt() == 1);
    case AggregatedDatumInfo::IHC_2:
        return (value.toInt() == 2);
    case AggregatedDatumInfo::IHC_3:
        return (value.toInt() == 3);
    case AggregatedDatumInfo::InvalidField:
        break;
    }
    return false;
}

QVariant DataAggregator::aggregate(const AggregatedDatumInfo& datumInfo,
                                   const QVariantList& values) const
{
    const int total = values.size();

    // For count, we dont care for the value, only for the existence of the property
    if (datumInfo.field == AggregatedDatumInfo::Count)
    {
        return total;
    }

    int aggregate = 0;
    foreach (const QVariant& value, values)
    {
        bool match = matches(datumInfo.field, field, value);

        switch (datumInfo.valueType)
        {
        case AggregatedDatumInfo::AbsoluteValue:
        case AggregatedDatumInfo::PercentageValue:
        case AggregatedDatumInfo::ConfidenceUpper:
        case AggregatedDatumInfo::ConfidenceLower:
            if (match) aggregate++;
        case AggregatedDatumInfo::InvalidValue:
            break;
        }
    }
    switch (datumInfo.valueType)
    {
    case AggregatedDatumInfo::AbsoluteValue:
        return aggregate;
    case AggregatedDatumInfo::PercentageValue:
        return double(aggregate)/double(total);
    case AggregatedDatumInfo::ConfidenceUpper:
    case AggregatedDatumInfo::ConfidenceLower:
    {
        ConfidenceInterval ci;
        ci.setEvents(aggregate);
        ci.setObservations(total);
        QPair<double,double> ciValues = ci.binomial();
        return (datumInfo.valueType == AggregatedDatumInfo::ConfidenceLower) ? ciValues.first : ciValues.second;
    }
    case AggregatedDatumInfo::InvalidValue:
        break;
    }
    return QVariant();
}

bool DataAggregator::isCountedAs(const Property& prop, const AggregatedDatumInfo& datumInfo) const
{
    if (!prop.isValid())
    {
        return false;
    }
    ValueTypeCategoryInfo fieldValueType(field);
    QVariant value = fieldValueType.toValue(prop.value);
    return matches(datumInfo.field, field, value);
}
