/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        , 21.02.2012
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

#ifndef PATHOLOGYPROPERTYINFO_H
#define PATHOLOGYPROPERTYINFO_H

// Qt includes

#include <QMetaType>
#include <QPair>
#include <QString>
#include <QVariant>

class IHCScore;
class Property;

class PathologyPropertyInfo
{
public:

    enum Property
    {
        InvalidProperty,
        IHC_PTEN,
        IHC_pAKT,
        IHC_pERK,
        IHC_ALK,
        IHC_HER2,
        IHC_HER2_DAKO,
        IHC_MLH1,
        IHC_MSH2,
        IHC_MSH6,
        IHC_pP70S6K,
        IHC_ER,
        IHC_PR,
        Fish_ALK,
        Fish_HER2,
        Fish_FGFR1,
        Fish_PIK3CA,
        Mut_KRAS_2,
        Mut_KRAS_3,
        Mut_EGFR_19_21,
        Mut_EGFR_18_20,
        Mut_PIK3CA_10_21,
        Mut_BRAF_15,
        Mut_BRAF_11,
        Mut_DDR2,
        Mut_PTEN,
        PCR_D5S346,
        PCR_BAT26,
        PCR_BAT25,
        PCR_D17S250,
        PCR_D2S123,
        Comb_HER2, // Reminder: Adjust LastProperty

        FirstProperty = IHC_PTEN,
        LastProperty  = Comb_HER2
    };

    enum ValueTypeCategory
    {
        InvalidCategory,
        IHCClassical,
        IHCClassicalPoints,
        IHCBoolean, // "<10%, niedrige Intensität"
        IHCBooleanPercentage,
        IHCTwoDim,
        Fish,
        Mutation,
        StableUnstable,
        BooleanCombination
    };

    PathologyPropertyInfo();
    PathologyPropertyInfo(Property property, ValueTypeCategory valueType,
                          const QString& id, const QString& label,
                          const QString& detailLabel = QString());
    PathologyPropertyInfo(Property property);
    bool isValid() const { return !id.isNull(); }
    bool operator<(const PathologyPropertyInfo& other) const
    { return (property == other.property) ? (valueType < other.valueType) : (property < other.property); }
    bool operator==(const PathologyPropertyInfo& other) const
    { return property == other.property && valueType == other.valueType; }

    Property          property;
    ValueTypeCategory valueType;
    QString           id;
    QString           label;
    QString           detailLabel;

    QString plainTextLabel() const;
    bool isIHC() const;
    bool isCombined() const;

    static PathologyPropertyInfo info(Property property);
    static PathologyPropertyInfo info(const QString& id);
};

class ValueTypeCategoryInfo
{
public:

    ValueTypeCategoryInfo(PathologyPropertyInfo::ValueTypeCategory category);
    ValueTypeCategoryInfo(const PathologyPropertyInfo& info);

    const PathologyPropertyInfo::ValueTypeCategory category;

    QList<QVariant> possibleValues() const;

    // Methods serving display and sorting in a model
    QString toString(const QVariant& value) const;
    QString toDisplayString(const Property& prop) const;
    QVariant toVariantData(const Property& prop) const;

    // Methods serving translation between UI, internal representation and database
    QString toPropertyValue(const QVariant& value) const;
    QVariant toValue(const QString& propertyValue) const;

    // Gives a special value depending on medical meaning (IHCScore), or toValue()
    QVariant toMedicalValue(const Property& prop) const;

    // Methods providing information for IHC value types
    bool isScored() const;
    bool isTwoDimScored() const;
    // If category is IHCTwoDim
    IHCScore toIHCScore(const Property& prop) const;

    bool hasDetail() const;

    // Prefix and suffix
    QPair<QString, QString> defaultDetailLabel() const;
};

class PathologyContextInfo
{
public:
public:

    enum Context
    {
        InvalidContext,
        Tumorprofil,
        BestRx,
        ColonRetrospektiv,
        ScreeningBGJ398,
        ScreeningBEZ235,
        ScreeningBKM120,

        FirstContext = Tumorprofil,
        LastContext  = ScreeningBKM120
    };

    PathologyContextInfo();
    PathologyContextInfo(Context context,
                          const QString& id, const QString& label);
    PathologyContextInfo(Context context);
    bool isValid() const { return !id.isNull(); }

    Context           context;
    QString           id;
    QString           label;

    static PathologyContextInfo info(Context context);
    static PathologyContextInfo info(const QString& id);
};

Q_DECLARE_METATYPE(PathologyPropertyInfo)
Q_DECLARE_METATYPE(PathologyContextInfo)

#endif // PATHOLOGYPROPERTYINFO_H
