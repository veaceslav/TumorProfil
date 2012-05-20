/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 21.02.2012
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

// Qt includes

#include <QObject>
#include <QRegExp>

// Local includes

#include "pathologypropertyinfo.h"

PathologyPropertyInfo::PathologyPropertyInfo()
    : property(InvalidProperty), valueType(InvalidCategory)
{
}

PathologyPropertyInfo::PathologyPropertyInfo(Property property, ValueTypeCategory valueType,
                                             const QString& id, const QString& label,
                                             const QString& detailLabel)
    : property(property), valueType(valueType), id(id), label(label), detailLabel(detailLabel)
{
}

PathologyPropertyInfo::PathologyPropertyInfo(Property property)
{
    *this = info(property);
}

enum ValueTypeCategory
{
    IHCClassical,
    IHCBoolean, // "<10%, niedrige Intensität"
    IHCBooleanPercentage,
    Fish,
    Mutation,
    StableUnstable
};

QString PathologyPropertyInfo::plainTextLabel() const
{
    if (label.contains('<'))
    {
        static QRegExp tagMatcher("<.+>");
        tagMatcher.setMinimal(true);
        QString plainLabel = label;
        plainLabel.replace(tagMatcher, "");
        return plainLabel;
    }
    return label;
}

bool PathologyPropertyInfo::isIHC() const
{
    return valueType == IHCClassical ||
           valueType == IHCBoolean ||
           valueType == IHCBooleanPercentage;
}

PathologyPropertyInfo PathologyPropertyInfo::info(Property property)
{
    switch (property)
    {
    case IHC_PTEN:
        return PathologyPropertyInfo(property, IHCBoolean, "ihc/pten", QObject::QObject::tr("PTEN"));
    case IHC_pAKT:
        return PathologyPropertyInfo(property, IHCClassical, "ihc/p-akt?p=s473", QObject::QObject::tr("<qt>p-AKT<sup>S473</sup></qt>"));
    case IHC_pERK:
        return PathologyPropertyInfo(property, IHCClassical, "ihc/p-erk", QObject::QObject::tr("p-ERK"));
    case IHC_ALK:
        return PathologyPropertyInfo(property, IHCClassical, "ihc/alk", QObject::QObject::tr("ALK"));
    case IHC_HER2:
        return PathologyPropertyInfo(property, IHCClassical, "ihc/her2", QObject::QObject::tr("HER2"));
    case Fish_ALK:
        return PathologyPropertyInfo(property, Fish, "fish/alk", QObject::QObject::tr("ALK-Translokation"), QObject::QObject::tr("Prozentsatz:"));
    case Fish_HER2:
        return PathologyPropertyInfo(property, Fish, "fish/her2", QObject::QObject::tr("HER2-Amplifikation"), QObject::QObject::tr("Ratio HER2/CEP7:"));
    case Mut_KRAS_2:
        return PathologyPropertyInfo(property, Mutation, "mut/kras?exon=2", QObject::QObject::tr("KRAS Exon 2"));
    case Mut_EGFR_19_21:
        return PathologyPropertyInfo(property, Mutation, "mut/egfr?exon=19,21", QObject::QObject::tr("EGFR Exon 19 & 21"));
    case Mut_PIK3CA_10_21:
        return PathologyPropertyInfo(property, Mutation, "mut/pik3ca?exon=10,21", QObject::QObject::tr("PIK3CA Exon 10 & 21"));
    case Mut_BRAF_15:
        return PathologyPropertyInfo(property, Mutation, "mut/braf?exon=15", QObject::QObject::tr("BRAF Exon 15"));
    case Mut_EGFR_18_20:
        return PathologyPropertyInfo(property, Mutation, "mut/egfr?exon=18,20", QObject::QObject::tr("EGFR Exon 18 & 20"));
    case Mut_KRAS_3:
        return PathologyPropertyInfo(property, Mutation, "mut/kras?exon=3", QObject::QObject::tr("KRAS Exon 3"));
    case Mut_BRAF_11:
        return PathologyPropertyInfo(property, Mutation, "mut/braf?exon=11", QObject::QObject::tr("BRAF Exon 11"));
    case Fish_FGFR1:
        return PathologyPropertyInfo(property, Fish, "fish/fgfr1", QObject::QObject::tr("FGFR1-Amplifikation"), QObject::QObject::tr("Ratio FGFR1/CEP8:"));
    case Fish_PIK3CA:
        return PathologyPropertyInfo(property, Fish, "fish/pik3ca", QObject::QObject::tr("PIK3ca-Amplifikation"));
    case Mut_DDR2:
        return PathologyPropertyInfo(property, Mutation, "mut/ddr2?exon=15-18", QObject::QObject::tr("DDR2 Exon 15-18"));
    case IHC_pP70S6K:
        return PathologyPropertyInfo(property, IHCBooleanPercentage, "ihc/p-p70S6k", QObject::QObject::tr("p-p70S6K"));
    case IHC_MLH1:
        return PathologyPropertyInfo(property, IHCClassical, "ihc/mlh1", QObject::QObject::tr("MLH1"));
    case IHC_MSH2:
        return PathologyPropertyInfo(property, IHCClassical, "ihc/msh2", QObject::QObject::tr("MSH2"));
    case IHC_MSH6:
        return PathologyPropertyInfo(property, IHCClassical, "ihc/msh6", QObject::QObject::tr("MSH6"));
    case PCR_D5S346:
        return PathologyPropertyInfo(property, StableUnstable, "msi/d5s346", QObject::QObject::tr("MSI PCR: D5S346"));
    case PCR_BAT26:
        return PathologyPropertyInfo(property, StableUnstable, "msi/bat26", QObject::QObject::tr("MSI PCR: BAT26"));
    case PCR_BAT25:
        return PathologyPropertyInfo(property, StableUnstable, "msi/bat25", QObject::QObject::tr("MSI PCR: BAT25"));
    case PCR_D17S250:
        return PathologyPropertyInfo(property, StableUnstable, "msi/d17s250", QObject::QObject::tr("MSI PCR: D17S250"));
    case PCR_D2S123:
        return PathologyPropertyInfo(property, StableUnstable, "msi/d2s123", QObject::QObject::tr("MSI PCR: D2S123"));
    case InvalidProperty:
        break;
    }
    return PathologyPropertyInfo();
}

PathologyPropertyInfo PathologyPropertyInfo::info(const QString& id)
{
    for (int i = FirstProperty; i<= LastProperty; i++)
    {
        PathologyPropertyInfo obj = info((Property)i);
        if (obj.id == id)
        {
            return obj;
        }
    }
    return PathologyPropertyInfo();
}


ValueTypeCategoryInfo::ValueTypeCategoryInfo(PathologyPropertyInfo::ValueTypeCategory category)
    : category(category)
{
}

ValueTypeCategoryInfo::ValueTypeCategoryInfo(const PathologyPropertyInfo& info)
    : category(info.valueType)
{
}

QList<QVariant> ValueTypeCategoryInfo::possibleValues() const
{
    QList<QVariant> values;
    switch (category)
    {
    case PathologyPropertyInfo::IHCClassical:
        values << QVariant(QVariant::Bool)
               << 0 << 1 << 2 << 3;
        break;
    case PathologyPropertyInfo::IHCBoolean:
        values << QVariant(QVariant::Bool) << false << true;
        break;
    case PathologyPropertyInfo::IHCBooleanPercentage:
        values << QVariant(QVariant::Bool) << false << true;
        break;
    case PathologyPropertyInfo::Fish:
        values << QVariant(QVariant::Bool) << false << true;
        break;
    case PathologyPropertyInfo::Mutation:
        values << QVariant(QVariant::Bool) << false << true;
        break;
    case PathologyPropertyInfo::StableUnstable:
        values << QVariant(QVariant::Bool) << false << true;
        break;
    case PathologyPropertyInfo::InvalidCategory:
        break;
    }
    return values;
}

bool ValueTypeCategoryInfo::isScored() const
{
    return category == PathologyPropertyInfo::IHCClassical;
}

QString ValueTypeCategoryInfo::toString(const QVariant& value) const
{
    if (category != PathologyPropertyInfo::InvalidCategory
            && value.isNull() && value.type() == QVariant::Bool)
    {
        return QObject::tr("n.d.");
    }

    switch (category)
    {
    case PathologyPropertyInfo::IHCClassical:
        switch (value.toInt())
        {
        case 0:
            return QObject::tr("0");
        case 1:
            return QObject::tr("1+");
        case 2:
            return QObject::tr("2+");
        case 3:
            return QObject::tr("3+");
        }
        break;
    case PathologyPropertyInfo::IHCBoolean:
        if (value.toBool())
        {
            return QObject::tr("positiv");
        }
        else
        {
            return QObject::tr("negativ (<10%, niedrige Intensität)");
        }
        break;
    case PathologyPropertyInfo::IHCBooleanPercentage:
    case PathologyPropertyInfo::Fish:
    case PathologyPropertyInfo::Mutation:
        if (value.toBool())
        {
            return QObject::tr("positiv");
        }
        else
        {
            if (category == PathologyPropertyInfo::Mutation)
            {
                return QObject::tr("Wildtyp");
            }
            else
            {
                return QObject::tr("negativ");
            }
        }
    case PathologyPropertyInfo::StableUnstable:
        if (value.toBool())
        {
            return QObject::tr("instabil");
        }
        else
        {
            return QObject::tr("stabil");
        }
    case PathologyPropertyInfo::InvalidCategory:
        break;
    }
    return QString();
}

QString ValueTypeCategoryInfo::toShortString(const QVariant& value) const
{
    if (category != PathologyPropertyInfo::InvalidCategory
            && value.isNull() && value.type() == QVariant::Bool)
    {
        return QString("");
    }

    switch (category)
    {
    case PathologyPropertyInfo::IHCClassical:
        switch (value.toInt())
        {
        case 0:
            return QObject::tr("0");
        case 1:
            return QObject::tr("1+");
        case 2:
            return QObject::tr("2+");
        case 3:
            return QObject::tr("3+");
        }
        break;
    case PathologyPropertyInfo::IHCBoolean:
    case PathologyPropertyInfo::IHCBooleanPercentage:
    case PathologyPropertyInfo::Fish:
    case PathologyPropertyInfo::Mutation:
    case PathologyPropertyInfo::StableUnstable:
        if (value.toBool())
        {
            return "+";
        }
        else
        {
            return "-";
        }
    case PathologyPropertyInfo::InvalidCategory:
        break;
    }
    return QString();
}

namespace
{

static QString boolToString(bool b)
{
    if (b)
    {
        return "1";
    }
    else
    {
        return "0";
    }
}

static QString boolToString(const QVariant& var)
{
    return boolToString(var.toBool());
}

static bool stringToBool(const QString& s)
{
    if (s == "1" || s == "true" || s == "pos" || s.startsWith("positiv") || s == "+")
    {
        return true;
    }
    if (s == "0" || s == "false" || s == "neg" || s.startsWith("negativ") || s == "-")
    {
        return false;
    }
    bool ok;
    int i = s.toInt(&ok);
    if (ok)
    {
        return i;
    }
    return false;
}
}

QString ValueTypeCategoryInfo::toPropertyValue(const QVariant& value) const
{
    switch (category)
    {
    case PathologyPropertyInfo::IHCClassical:
        return value.toString();
        break;
    case PathologyPropertyInfo::IHCBoolean:
    case PathologyPropertyInfo::IHCBooleanPercentage:
    case PathologyPropertyInfo::Fish:
    case PathologyPropertyInfo::Mutation:
    case PathologyPropertyInfo::StableUnstable:
        return boolToString(value);
    case PathologyPropertyInfo::InvalidCategory:
        break;
    }
    return QString();
}

QVariant ValueTypeCategoryInfo::toValue(const QString& propertyValue) const
{
    switch (category)
    {
    case PathologyPropertyInfo::IHCClassical:
        return propertyValue.toInt();
        break;
    case PathologyPropertyInfo::IHCBoolean:
    case PathologyPropertyInfo::IHCBooleanPercentage:
    case PathologyPropertyInfo::Fish:
    case PathologyPropertyInfo::Mutation:
    case PathologyPropertyInfo::StableUnstable:
        return stringToBool(propertyValue);
        break;
    case PathologyPropertyInfo::InvalidCategory:
        break;
    }
    return QVariant();
}


bool ValueTypeCategoryInfo::hasDetail() const
{
    switch (category)
    {
    case PathologyPropertyInfo::IHCBooleanPercentage:
    case PathologyPropertyInfo::Fish:
    case PathologyPropertyInfo::Mutation:
        return true;
    default:
        return false;
    }
}

QPair<QString, QString> ValueTypeCategoryInfo::defaultDetailLabel() const
{
    switch (category)
    {
    case PathologyPropertyInfo::IHCClassical:
    case PathologyPropertyInfo::IHCBoolean:
    case PathologyPropertyInfo::StableUnstable:
    case PathologyPropertyInfo::InvalidCategory:
        break;
    case PathologyPropertyInfo::IHCBooleanPercentage:
        return qMakePair(QString(), QObject::tr("% Zellen"));
        break;
    case PathologyPropertyInfo::Fish:
        return qMakePair(QString(""), QString());
        break;
    case PathologyPropertyInfo::Mutation:
        return qMakePair(QObject::tr("Mutation:"), QString());
        break;
    }
    return QPair<QString, QString>();
}

PathologyContextInfo::PathologyContextInfo()
    : context(InvalidContext)
{
}

PathologyContextInfo::PathologyContextInfo(Context context,
                      const QString& id, const QString& label)
    : context(context), id(id), label(label)
{
}

PathologyContextInfo::PathologyContextInfo(Context context)
{
    *this = info(context);
}

PathologyContextInfo PathologyContextInfo::info(Context context)
{
    switch (context)
    {
    case InvalidContext:
        break;
    case Tumorprofil:
        return PathologyContextInfo(context, "wtz/tumorprofil", "Tumorprofil");
    case BestRx:
        return PathologyContextInfo(context, "novartis/bestrx", "BestRx");
    case ColonRetrospektiv:
        return PathologyContextInfo(context, "wtz/kasper/colonretrospektiv", "Colon retrospektiv");
    case ScreeningBGJ398:
        return PathologyContextInfo(context, "novartis/CBGJ398X2101", "BGJ398");
    case ScreeningBEZ235:
        return PathologyContextInfo(context, "novartis/CBEZ235A2101", "CBEZ235");
    }
    return PathologyContextInfo();
}

PathologyContextInfo PathologyContextInfo::info(const QString& id)
{
    for (int i = FirstContext; i<= LastContext; i++)
    {
        PathologyContextInfo obj = info((Context)i);
        if (obj.id == id)
        {
            return obj;
        }
    }
    return PathologyContextInfo();
}
