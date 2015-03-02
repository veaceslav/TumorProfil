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

#include "ihcscore.h"
#include "pathologypropertyinfo.h"
#include "property.h"


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
           valueType == IHCClassicalPoints ||
           valueType == IHCBoolean ||
           valueType == IHCBooleanPercentage ||
           valueType == IHCTwoDim ||
           valueType == IHCHScore;
}

bool PathologyPropertyInfo::isCombined() const
{
    return valueType == BooleanCombination;
}

PathologyPropertyInfo PathologyPropertyInfo::info(Property property)
{
    switch (property)
    {
    case IHC_PTEN:
        return PathologyPropertyInfo(property, IHCTwoDim, "ihc/pten", QObject::tr("PTEN"));
    case IHC_pAKT:
        return PathologyPropertyInfo(property, IHCTwoDim, "ihc/p-akt?p=s473", QObject::tr("<qt>p-AKT<sup>S473</sup></qt>"));
    case IHC_pERK:
        return PathologyPropertyInfo(property, IHCTwoDim, "ihc/p-erk", QObject::tr("p-ERK"));
    case IHC_ALK:
        return PathologyPropertyInfo(property, IHCTwoDim, "ihc/alk", QObject::tr("ALK"));
    case IHC_HER2:
        return PathologyPropertyInfo(property, IHCTwoDim, "ihc/her2?score=twodim", QObject::tr("HER2"));
    case IHC_HER2_DAKO:
        return PathologyPropertyInfo(property, IHCClassical, "ihc/her2", QObject::tr("HER2 DAKO"));
    case IHC_ER:
        return PathologyPropertyInfo(property, IHCClassical, "ihc/er", QObject::tr("ER"));
    case IHC_PR:
        return PathologyPropertyInfo(property, IHCClassical, "ihc/pr", QObject::tr("PR"));
    case IHC_cMET:
        return PathologyPropertyInfo(property, IHCHScore, "ihc/c-met", QObject::tr("cMet"));
    case IHC_ROS1:
        return PathologyPropertyInfo(property, IHCTwoDim, "ihc/ros1", QObject::tr("ROS1"));
    case Fish_ALK:
        return PathologyPropertyInfo(property, Fish, "fish/alk", QObject::tr("ALK-Translokation"), QObject::tr("Prozentsatz:"));
    case Fish_HER2:
        return PathologyPropertyInfo(property, Fish, "fish/her2", QObject::tr("HER2-Amplifikation"), QObject::tr("Ratio HER2/CEP7:"));
    case Mut_KRAS_2:
        return PathologyPropertyInfo(property, Mutation, "mut/kras?exon=2", QObject::tr("KRAS Exon 2"));
    case Mut_EGFR_19_21:
        return PathologyPropertyInfo(property, Mutation, "mut/egfr?exon=19,21", QObject::tr("EGFR Exon 19 & 21"));
    case Mut_PIK3CA_10_21:
        return PathologyPropertyInfo(property, Mutation, "mut/pik3ca?exon=10,21", QObject::tr("PIK3CA Exon 10 & 21"));
    case Mut_BRAF_15:
        return PathologyPropertyInfo(property, Mutation, "mut/braf?exon=15", QObject::tr("BRAF Exon 15"));
    case Mut_EGFR_18_20:
        return PathologyPropertyInfo(property, Mutation, "mut/egfr?exon=18,20", QObject::tr("EGFR Exon 18 & 20"));
    case Mut_KRAS_3:
        return PathologyPropertyInfo(property, Mutation, "mut/kras?exon=3", QObject::tr("KRAS Exon 3"));
    case Mut_KRAS_4:
        return PathologyPropertyInfo(property, Mutation, "mut/kras?exon=4", QObject::tr("KRAS Exon 4"));
    case Mut_NRAS_2_4:
        return PathologyPropertyInfo(property, Mutation, "mut/nras?exon=2-4", QObject::tr("NRAS Exon 2-4"));
    case Mut_BRAF_11:
        return PathologyPropertyInfo(property, Mutation, "mut/braf?exon=11", QObject::tr("BRAF Exon 11"));
    case Fish_FGFR1:
        return PathologyPropertyInfo(property, Fish, "fish/fgfr1", QObject::tr("FGFR1-Amplifikation"), QObject::tr("Ratio FGFR1/CEP8:"));
    case Fish_PIK3CA:
        return PathologyPropertyInfo(property, Fish, "fish/pik3ca", QObject::tr("PIK3ca-Amplifikation"));
    case Fish_cMET:
        return PathologyPropertyInfo(property, Fish, "fish/c-met", QObject::tr("cMET-Amplifikation"));
    case Fish_ROS1:
        return PathologyPropertyInfo(property, Fish, "fish/ros1", QObject::tr("ROS1-Rearrangement"));
    case Mut_DDR2:
        return PathologyPropertyInfo(property, Mutation, "mut/ddr2?exon=15-18", QObject::tr("DDR2 Exon 15-18"));
    case Mut_PTEN:
        return PathologyPropertyInfo(property, Mutation, "mut/pten", QObject::tr("PTEN"));
    case IHC_pP70S6K:
        return PathologyPropertyInfo(property, IHCTwoDim, "ihc/p-p70S6k", QObject::tr("p-p70S6K"));
    case IHC_MLH1:
        return PathologyPropertyInfo(property, IHCClassical, "ihc/mlh1", QObject::tr("MLH1"));
    case IHC_MSH2:
        return PathologyPropertyInfo(property, IHCClassical, "ihc/msh2", QObject::tr("MSH2"));
    case IHC_MSH6:
        return PathologyPropertyInfo(property, IHCClassical, "ihc/msh6", QObject::tr("MSH6"));
    case PCR_D5S346:
        return PathologyPropertyInfo(property, StableUnstable, "msi/d5s346", QObject::tr("MSI PCR: D5S346"));
    case PCR_BAT26:
        return PathologyPropertyInfo(property, StableUnstable, "msi/bat26", QObject::tr("MSI PCR: BAT26"));
    case PCR_BAT25:
        return PathologyPropertyInfo(property, StableUnstable, "msi/bat25", QObject::tr("MSI PCR: BAT25"));
    case PCR_D17S250:
        return PathologyPropertyInfo(property, StableUnstable, "msi/d17s250", QObject::tr("MSI PCR: D17S250"));
    case PCR_D2S123:
        return PathologyPropertyInfo(property, StableUnstable, "msi/d2s123", QObject::tr("MSI PCR: D2S123"));
    case Comb_HER2:
        return PathologyPropertyInfo(property, BooleanCombination, "combination/her2", QObject::tr("HER2-Status"));
    case Comb_HormoneReceptor:
        return PathologyPropertyInfo(property, BooleanCombination, "combination/er_pr", QObject::tr("HR-Status"));
    case Comb_TripleNegative:
        return PathologyPropertyInfo(property, BooleanCombination, "combination/triplenegative", QObject::tr("Triple-neg."));
    case Comb_cMetActivation:
        return PathologyPropertyInfo(property, BooleanCombination, "combination/c-met", QObject::tr("cMet-Aktivierung"));
    case Comb_cMetIHC3plusScore:
        return PathologyPropertyInfo(property, BooleanCombination, "combination/c-met/ihc3+score", QObject::tr("cMet IHC 3+ Score"));
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

QList<QVariant> ValueTypeCategoryInfo::optionsInUI() const
{
    QList<QVariant> values;
    switch (category)
    {
    case PathologyPropertyInfo::IHCClassical:
    case PathologyPropertyInfo::IHCClassicalPoints:
    case PathologyPropertyInfo::IHCTwoDim:
    case PathologyPropertyInfo::IHCHScore:
        // the latter receives special treatment
        values << QVariant(QVariant::Bool)
               << 0 << 1 << 2 << 3;
        break;
    case PathologyPropertyInfo::IHCBoolean:
    case PathologyPropertyInfo::IHCBooleanPercentage:
    case PathologyPropertyInfo::Fish:
    case PathologyPropertyInfo::Mutation:
    case PathologyPropertyInfo::StableUnstable:
    case PathologyPropertyInfo::BooleanCombination:
        values << QVariant(QVariant::Bool) << false << true;
        break;
    case PathologyPropertyInfo::InvalidCategory:
        break;
    }
    return values;
}

bool ValueTypeCategoryInfo::isScored() const
{
    return category == PathologyPropertyInfo::IHCClassical ||
           category == PathologyPropertyInfo::IHCTwoDim ||
           category == PathologyPropertyInfo::IHCClassicalPoints ||
           category == PathologyPropertyInfo::IHCHScore;
}

bool ValueTypeCategoryInfo::isTwoDimScored() const
{
    return category == PathologyPropertyInfo::IHCTwoDim;
}

bool ValueTypeCategoryInfo::isHScored() const
{
    return category == PathologyPropertyInfo::IHCHScore;
}

QString ValueTypeCategoryInfo::toUILabel(const QVariant& value) const
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
    case PathologyPropertyInfo::IHCClassicalPoints:
        return QString::number(value.toInt()) + (value.toInt() == 0 ? QString() : "  ");
                // blank to have same length as IHC classical
    case PathologyPropertyInfo::IHCTwoDim:
    case PathologyPropertyInfo::IHCHScore:
        switch (value.toInt())
        {
        case 0:
            return QObject::tr("keine");
        case 1:
            return QObject::tr("schwache");
        case 2:
            return QObject::tr("mäßige");
        case 3:
            return QObject::tr("starke Färbung");
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
    case PathologyPropertyInfo::BooleanCombination:
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

IHCScore ValueTypeCategoryInfo::toIHCScore(const Property& prop) const
{
    if (category != PathologyPropertyInfo::IHCTwoDim)
    {
        return IHCScore();
    }
    return IHCScore(toValue(prop.value), prop.detail);
}

HScore ValueTypeCategoryInfo::toHScore(const Property& prop) const
{
    if (category != PathologyPropertyInfo::IHCHScore)
    {
        return HScore();
    }
    return HScore(toValue(prop.value));
}

namespace
{

static QString toScoreString(int score)
{
    switch (score)
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
    return QString();
}

static QString toPlusMinus(bool val)
{
    if (val)
    {
        return "+";
    }
    else
    {
        return "-";
    }
}

}

// SELECT property FROM PathologyProperties WHERE pathologyid IN (SELECT id FROM Pathologies WHERE context='wtz/tumorprofil') AND property LIKE 'ihc/%';
QString ValueTypeCategoryInfo::toDisplayString(const Property& prop) const
{
    QVariant value = toValue(prop.value);

    if (category != PathologyPropertyInfo::InvalidCategory
            && value.isNull() && value.type() == QVariant::Bool)
    {
        return QString("");
    }

    switch (category)
    {
    case PathologyPropertyInfo::IHCClassical:
        return toScoreString(value.toInt());
    case PathologyPropertyInfo::IHCClassicalPoints:
        return QString::number(value.toInt());
    case PathologyPropertyInfo::IHCTwoDim:
    {
        QVariant score = IHCScore(value, prop.detail).score();
        if (score.isNull())
        {
            return QString();
        }
        else if (score.type() == QVariant::Int)
        {
            return QString::number(score.toInt());
        }
        else
        {
            return toPlusMinus(score.toBool());
        }
    }
    case PathologyPropertyInfo::IHCHScore:
    {
        QVariant score = HScore(value).score();
        if (score.isNull())
        {
            return QString();
        }
        return QString::number(score.toInt());
    }
    case PathologyPropertyInfo::IHCBoolean:
    case PathologyPropertyInfo::IHCBooleanPercentage:
    case PathologyPropertyInfo::Fish:
    case PathologyPropertyInfo::Mutation:
    case PathologyPropertyInfo::StableUnstable:
    case PathologyPropertyInfo::BooleanCombination:
        return toPlusMinus(value.toBool());
    case PathologyPropertyInfo::InvalidCategory:
        break;
    }
    return QString();
}

QVariant ValueTypeCategoryInfo::toVariantData(const Property& prop) const
{
    QVariant value = toValue(prop.value);

    if (category != PathologyPropertyInfo::InvalidCategory
            && value.isNull() && value.type() == QVariant::Bool)
    {
        return QVariant();
    }

    switch (category)
    {
    case PathologyPropertyInfo::IHCClassical:
    case PathologyPropertyInfo::IHCClassicalPoints:
        return value;
    case PathologyPropertyInfo::IHCTwoDim:
    {
        QVariant score = IHCScore(value, prop.detail).score();
        if (score.isNull())
        {
            return QVariant();
        }
        else if (score.type() == QVariant::Int)
        {
            return score;
        }
        else
        {
            return score.toBool() ? 10 : 0;
        }
    }
    case PathologyPropertyInfo::IHCHScore:
    {
        QVariant score = HScore(value).score();
        if (score.isNull())
        {
            return QVariant();
        }
        return score;
    }
    case PathologyPropertyInfo::IHCBoolean:
    case PathologyPropertyInfo::IHCBooleanPercentage:
    case PathologyPropertyInfo::Fish:
    case PathologyPropertyInfo::Mutation:
    case PathologyPropertyInfo::StableUnstable:
    case PathologyPropertyInfo::BooleanCombination:
        return value;
    case PathologyPropertyInfo::InvalidCategory:
        break;
    }
    return QVariant();
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
    case PathologyPropertyInfo::IHCClassicalPoints:
    case PathologyPropertyInfo::IHCTwoDim:
    case PathologyPropertyInfo::IHCHScore:
        return value.toString();
        break;
    case PathologyPropertyInfo::IHCBoolean:
    case PathologyPropertyInfo::IHCBooleanPercentage:
    case PathologyPropertyInfo::Fish:
    case PathologyPropertyInfo::Mutation:
    case PathologyPropertyInfo::StableUnstable:
    case PathologyPropertyInfo::BooleanCombination:
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
    case PathologyPropertyInfo::IHCClassicalPoints:
    case PathologyPropertyInfo::IHCTwoDim:
    case PathologyPropertyInfo::IHCHScore:
        return propertyValue.toInt();
        break;
    case PathologyPropertyInfo::IHCBoolean:
    case PathologyPropertyInfo::IHCBooleanPercentage:
    case PathologyPropertyInfo::Fish:
    case PathologyPropertyInfo::Mutation:
    case PathologyPropertyInfo::StableUnstable:
    case PathologyPropertyInfo::BooleanCombination:
        return stringToBool(propertyValue);
        break;
    case PathologyPropertyInfo::InvalidCategory:
        break;
    }
    return QVariant();
}

QVariant ValueTypeCategoryInfo::toMedicalValue(const Property &prop) const
{
    if (isTwoDimScored())
    {
        return QVariant::fromValue(toIHCScore(prop));
    }
    else if (isHScored())
    {
        return QVariant::fromValue(toHScore(prop));
    }
    return toValue(prop.value);
}

bool ValueTypeCategoryInfo::hasDetail() const
{
    switch (category)
    {
    case PathologyPropertyInfo::IHCBooleanPercentage:
    case PathologyPropertyInfo::IHCTwoDim:
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
    case PathologyPropertyInfo::IHCClassicalPoints:
    case PathologyPropertyInfo::IHCBoolean:
    case PathologyPropertyInfo::StableUnstable:
    case PathologyPropertyInfo::InvalidCategory:
    case PathologyPropertyInfo::BooleanCombination:
    case PathologyPropertyInfo::IHCHScore:
        break;
    case PathologyPropertyInfo::IHCBooleanPercentage:
    case PathologyPropertyInfo::IHCTwoDim:
        return qMakePair(QObject::tr("in"), QObject::tr("% Zellen"));
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

// -----------------

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
    case ScreeningBKM120:
        return PathologyContextInfo(context, "novartis/CBKM120D2201", "CBEZ235");
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


// ------------

TrialContextInfo::TrialContextInfo()
    : trial(InvalidTrial)
{
}

TrialContextInfo::TrialContextInfo(Trial trial,
                      const QString& id, const QString& label)
    : trial(trial), id(id), label(label)
{
}

TrialContextInfo::TrialContextInfo(Trial trial)
{
    *this = info(trial);
}

TrialContextInfo TrialContextInfo::info(Trial trial)
{
    switch (trial)
    {
    case InvalidTrial:
        break;
    case AIO_TRK_0212:
        return TrialContextInfo(trial, "aio/TRK-0212", "PemSplitCisp Studie");
    }
    return TrialContextInfo();
}

TrialContextInfo TrialContextInfo::info(const QString& id)
{
    for (int i = FirstTrial; i<= LastTrial; i++)
    {
        TrialContextInfo obj = info((Trial)i);
        if (obj.id == id)
        {
            return obj;
        }
    }
    return TrialContextInfo();
}

