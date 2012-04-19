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

#include <QPair>
#include <QString>
#include <QVariant>

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
        IHC_MLH1,
        IHC_MSH2,
        IHC_MSH6,
        IHC_pP70S6K,
        Fish_ALK,
        Fish_HER2,
        Fish_FGFR1,
        Mut_KRAS_2,
        Mut_KRAS_3,
        Mut_EGFR_19_21,
        Mut_EGFR_18_20,
        Mut_PIK3CA_10_21,
        Mut_BRAF_15,
        Mut_BRAF_11,
        Mut_DDR2,
        PCR_D5S346,
        PCR_BAT26,
        PCR_BAT25,
        PCR_D17S250,
        PCR_D2S123,

        FirstProperty = IHC_PTEN,
        LastProperty  = PCR_D2S123
    };

    enum ValueTypeCategory
    {
        InvalidCategory,
        IHCClassical,
        IHCBoolean, // "<10%, niedrige Intensität"
        IHCBooleanPercentage,
        Fish,
        Mutation,
        StableUnstable
    };

    PathologyPropertyInfo();
    PathologyPropertyInfo(Property property, ValueTypeCategory valueType,
                          const QString& id, const QString& label,
                          const QString& detailLabel = QString());
    PathologyPropertyInfo(Property property);

    Property          property;
    ValueTypeCategory valueType;
    QString           id;
    QString           label;
    QString           detailLabel;

    QString plainTextLabel() const;
    bool isIHC() const;

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

    QString toString(const QVariant& value) const;
    QString toShortString(const QVariant& value) const;

    QString toPropertyValue(const QVariant& value) const;
    QVariant toValue(const QString& propertyValue) const;

    bool hasDetail() const;
    // Prefix and suffix
    QPair<QString, QString> defaultDetailLabel() const;
};

#endif // PATHOLOGYPROPERTYINFO_H
