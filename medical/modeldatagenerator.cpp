/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 12.04.2012
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

#include <QDebug>

// Local includes

#include "modeldatagenerator.h"
#include "patientmodel.h"
#include "resultcompletenesschecker.h"

ModelDataGenerator::ModelDataGenerator(const QList<PathologyPropertyInfo>&  infos,
                                       const Patient::Ptr& p, int role, int field)
    : p(p),
      hasPathology(p ? p->hasPathology() : false),
      field(field),
      role(role),
      infos(infos)
{
}

QVariant ModelDataGenerator::entityNameDatum()
{
    if (role == PatientModel::VariantDataRole)
    {
        return p->firstDisease().entity();
    }
    switch (p->firstDisease().entity())
    {
    case Pathology::PulmonaryAdeno:
        return QObject::tr("Lunge Adeno");
    case Pathology::PulmonaryBronchoalveloar:
        return QObject::tr("Lunge Bronchioloaveolär");
    case Pathology::PulmonaryLargeCell:
        return QObject::tr("Lunge Großzeller");
    case Pathology::PulmonarySquamous:
        return QObject::tr("Lunge Platte");
    case Pathology::PulmonaryAdenosquamous:
        return QObject::tr("Lunge Adenosquamös");
    case Pathology::PulmonaryOtherCarcinoma:
        return QObject::tr("Lunge anderes");
    case Pathology::ColorectalAdeno:
        return QObject::tr("Kolorektal");
    case Pathology::Cholangiocarcinoma:
        return QObject::tr("Gallengang");
    case Pathology::RenalCell:
        return QObject::tr("Nierenzell");
    case Pathology::Esophageal:
        return QObject::tr("Ösophagus");
    case Pathology::EsophagogastrealJunction:
        return QObject::tr("Ösophagogastral");
    case Pathology::Gastric:
        return QObject::tr("Magen");
    case Pathology::Breast:
        return QObject::tr("Mamma");
    case Pathology::TransitionalCell:
        return QObject::tr("Urothel");
    case Pathology::Thyroid:
        return QObject::tr("Schilddrüse");
    case Pathology::Melanoma:
        return QObject::tr("Melanom");
    case Pathology::UnknownEntity:
    default:
        return QString();
    }
}

QVariant ModelDataGenerator::fieldDatum()
{
    const PathologyPropertyInfo& info = infos.at(field);
    Property prop = p->firstDisease().pathologyProperty(info.id);
    if (prop.isNull())
    {
        return QString();
    }
    ValueTypeCategoryInfo typeInfo(info.valueType);
    QVariant value = typeInfo.toValue(prop.value);
    if (role == PatientModel::VariantDataRole)
    {
        return value;
    }
    if (info.valueType == PathologyPropertyInfo::Mutation &&
            value.toBool() && !prop.detail.isEmpty())
    {
        return prop.detail;
    }
    return typeInfo.toShortString(value);
}

QVariant ModelDataGenerator::fieldHeader()
{
    return infos.at(field).plainTextLabel();
}

QVariant ModelDataGenerator::otherMutationsDatum()
{
    QString data;
    foreach (const Property& prop, p->firstDisease().allPathologyProperties())
    {
        bool isInList = false;
        foreach (const PathologyPropertyInfo& info, infos)
        {
            if (info.id == prop.property)
            {
                isInList = true;
                break;
            }
        }
        if (isInList)
        {
            continue;
        }
        qDebug() << "Not in list:" << prop.property << prop.value;
        PathologyPropertyInfo info = PathologyPropertyInfo::info(prop.property);
        ValueTypeCategoryInfo typeInfo(info.valueType);
        QVariant value = typeInfo.toValue(prop.value);
        if (value.toBool())
        {
            if (!data.isEmpty())
                data += "; ";
            data += info.plainTextLabel() + ": " + typeInfo.toShortString(value);
        }
    }
    return data;
}

QVariant ModelDataGenerator::otherMutationsHeader()
{
    return QObject::tr("Weitere Befunde");
}

QVariant ModelDataGenerator::completenessDatum(CompletenessField value)
{
    if (!hasPathology)
    {
        return QObject::tr("Keine Befunde");
    }

    ResultCompletenessChecker checker(p);
    ResultCompletenessChecker::CompletenessResult result;
    QList<PathologyPropertyInfo> missingProperties;

    switch (value)
    {
    case IHCCompleteness:
        result = checker.isIHCComplete(&missingProperties);
        break;
    case MutationCompleteness:
        result = checker.isMutComplete(&missingProperties);
        break;
    default:
        return QVariant();
    }

    if (role == Qt::DisplayRole)
    {
        switch (result)
        {
        case ResultCompletenessChecker::Complete:
            return QObject::tr("ja");
        case ResultCompletenessChecker::Incomplete:
            break;
        case ResultCompletenessChecker::Absent:
            return QObject::tr("fehlt");
        case ResultCompletenessChecker::Undefined:
        case ResultCompletenessChecker::OnlyOptional:
            return QObject::tr("N/A");
        }

        // Incomplete: which fields are missing?
        QString missing;
        foreach (const PathologyPropertyInfo& info, missingProperties)
        {
            if (!missing.isEmpty())
                missing += ", ";
            missing += info.plainTextLabel();
        }
        missing += (missingProperties.size() > 1) ? QObject::tr(" fehlen") : QObject::tr(" fehlt");
        return missing;
    }
    else // VariantRole
    {
        switch (result)
        {
        case ResultCompletenessChecker::Complete:
            return true;
        case ResultCompletenessChecker::Incomplete:
            return false;
        case ResultCompletenessChecker::Absent:
            return false;
        case ResultCompletenessChecker::Undefined:
        case ResultCompletenessChecker::OnlyOptional:
            return true;
        }
    }
    return QVariant();

    /*
    int all = 0;
    int available = 0;

    foreach (const PathologyPropertyInfo& info, infos)
    {
        switch (value)
        {
        case IHCCompleteness:
            if (!info.isIHC())
                continue;
            break;
        case MutationCompleteness:
            if (info.valueType != PathologyPropertyInfo::Mutation)
                continue;
            break;
        }

        Property prop = p->firstPathology().properties.property(info.id);
        if (!prop.isEmpty())
        {
            available++;
        }
        all++;
    }
    return (role == Qt::DisplayRole) ?
            QVariant(QString("%1/%2").arg(available).arg(all)) :
            QVariant((qreal(available) / qreal(qMax(1,all))));
    */
}

QVariant ModelDataGenerator::completenessHeader(CompletenessField value)
{
    switch (field)
    {
    case IHCCompleteness:
        return QObject::tr("IHC vorliegend");
    case MutationCompleteness:
        return QObject::tr("Mut. vorliegend");
    }
    return QVariant();
}
