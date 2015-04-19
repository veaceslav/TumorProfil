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

#include "actionableresultchecker.h"
#include "combinedvalue.h"
#include "dataaggregator.h"
#include "modeldatagenerator.h"
#include "patientmodel.h"
#include "patientpropertymodel.h"
#include "resultcompletenesschecker.h"

ModelDataGenerator::ModelDataGenerator(const QList<PathologyPropertyInfo>&  infos,
                                       const Patient::Ptr& p, int role, int field,
                                       PathologyContextInfo::Context primaryContext)
    : p(p),
      hasPathology(p ? p->hasPathology() : false),
      field(field),
      role(role),
      infos(infos),
      primaryContext(primaryContext)
{
}

QVariant ModelDataGenerator::entityNameDatum()
{
    switch (role)
    {
    case PatientModel::VariantDataRole:
        return p->firstDisease().entity();
    case Qt::DisplayRole:
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
    return QVariant();
}

namespace
{
}

QVariant ModelDataGenerator::returnDate(const QDate& date)
{
    if (role == Qt::DisplayRole)
    {
        if (date.isNull())
        {
            return QString();
        }
        return date.toString(QObject::tr("dd.MM.yyyy"));
    }
    else
    {
        return date;
    }
}

QVariant ModelDataGenerator::additionalInfoDatum()
{
    if (!p->hasDisease())
    {
        return QVariant();
    }
    switch (role)
    {
    case Qt::DisplayRole:
    case PatientPropertyModel::VariantDataRole:
        switch (field)
        {
        case GenderColumn:
            if (role == Qt::DisplayRole)
            {
                switch (p->gender)
                {
                case Patient::Male:
                    return "m";
                case Patient::Female:
                    return "w";
                case Patient::UnknownGender:
                    return "?";
                }
            }
            else
            {
                return p->gender;
            }
            break;
        case AgeColumn:
            return p->dateOfBirth.daysTo(QDate::currentDate())/ 365;
        case InitialDiagnosisColumn:
            return returnDate(p->firstDisease().initialDiagnosis);
        case DiseaseAgeColumn:
            return p->firstDisease().initialDiagnosis.daysTo(QDate::currentDate()) / 30.0;
        case ResultDateColumn:
        {
            if (p->firstDisease().hasPathology(primaryContext))
            {
                return returnDate(p->firstDisease().firstPathology(primaryContext).date);
            }
            break;
        }
        case ResultLocationColumn:
        {
            if (p->firstDisease().hasPathology(primaryContext))
            {
                switch (p->firstDisease().firstPathology(primaryContext).sampleOrigin)
                {
                case Pathology::Primary:
                    return QObject::tr("Primärtumor");
                case Pathology::LocalLymphNode:
                    return QObject::tr("Lymphknoten");
                case Pathology::Metastasis:
                    return QObject::tr("Metastase");
                case Pathology::UnknownOrigin:
                    return QString();
                }
            }
            break;
        }
        case DiseaseAgeAtResultColumn:
            if (p->firstDisease().hasPathology(primaryContext))
            {
                QDate resultDate = p->firstDisease().firstPathology(primaryContext).date;
                if (!resultDate.isValid())
                    return QVariant();
                int days = p->firstDisease().initialDiagnosis.daysTo(resultDate);
                if (days < 50)
                    return 0;
                return days / 30;
            }
            break;
        }
        break;
    }
    return QVariant();
}

QVariant ModelDataGenerator::additionalInfoHeader()
{
    switch (role)
    {
    case Qt::DisplayRole:
        switch (field)
        {
        case GenderColumn:
            return QObject::tr("M/W");
        case AgeColumn:
            return QObject::tr("Alter");
        case InitialDiagnosisColumn:
            return QObject::tr("ED");
        case DiseaseAgeColumn:
            return QObject::tr("Monate seit ED");
        case ResultDateColumn:
            return QObject::tr("Befund vom");
        case ResultLocationColumn:
            return QObject::tr("Befund aus");
        case DiseaseAgeAtResultColumn:
            return QObject::tr("nach (Monaten)");
        }
    case PatientPropertyModel::DataAggregationNatureRole:
        switch (field)
        {
        case GenderColumn:
            return DataAggregation::Gender;
        case AgeColumn:
        case DiseaseAgeColumn:
        case DiseaseAgeAtResultColumn:
            return DataAggregation::Numeric;
        case InitialDiagnosisColumn:
        case ResultDateColumn:
        case ResultLocationColumn:
            return DataAggregation::Date;
        }
    default:
        break;
    }
    return QVariant();
}

QVariant ModelDataGenerator::fieldDatum()
{
    const PathologyPropertyInfo& info = infos.at(field);
    Property prop;
    QString combinedString;
    QVariant combinedVariant;
    if (info.isCombined())
    {
        CombinedValue combinedValue(info);
        combinedValue.combine(p->firstDisease());
        prop = combinedValue.result();
        if (role == Qt::DisplayRole)
        {
            combinedString = combinedValue.toDisplayString();
        }
        else if (role == PatientModel::VariantDataRole)
        {
            combinedVariant = combinedValue.toCombinedVariant();
        }
    }
    else
    {
        prop = p->firstDisease().pathologyProperty(info.id);
    }

    if (prop.isNull())
    {
        return QVariant();
    }

    ValueTypeCategoryInfo typeInfo(info.valueType);

    switch (role)
    {
    case PatientModel::VariantDataRole:
        if (info.isCombined())
        {
            return combinedVariant;
        }
        return typeInfo.toVariantData(prop);
    case PatientPropertyModel::PathologyPropertyRole:
        return QVariant::fromValue(prop);
    case Qt::DisplayRole:
        if (info.valueType == PathologyPropertyInfo::Mutation)
        {
            QVariant value = typeInfo.toValue(prop.value);
            if (value.toBool() && !prop.detail.isEmpty())
            {
                return prop.detail;
            }
        }
        else if (info.isCombined())
        {
            return combinedString;
        }
        return typeInfo.toDisplayString(prop);
    }
    return QVariant();
}

QVariant ModelDataGenerator::fieldHeader()
{
    switch (role)
    {
    case Qt::DisplayRole:
        return infos.at(field).plainTextLabel();
    case PatientPropertyModel::PathologyPropertyInfoRole:
        return QVariant::fromValue(infos.at(field));
    case PatientPropertyModel::DataAggregationNatureRole:
        return DataAggregation::PathologyResult;
    default:
        return QVariant();
    }
}

QVariant ModelDataGenerator::otherMutationsDatum()
{
    QList<QString> blacklist;
    blacklist << PathologyPropertyInfo(PathologyPropertyInfo::IHC_HER2).id;

    switch (role)
    {
    case Qt::DisplayRole:
    case PatientModel::VariantDataRole:
        QString data;
        foreach (const Property& prop, p->firstDisease().allPathologyProperties())
        {
            if (blacklist.contains(prop.property))
            {
                continue;
            }
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
            //qDebug() << "Not in list:" << prop.property << prop.value;
            PathologyPropertyInfo info = PathologyPropertyInfo::info(prop.property);
            ValueTypeCategoryInfo typeInfo(info.valueType);
            QVariant value = typeInfo.toValue(prop.value);
            if (value.toBool())
            {
                if (!data.isEmpty())
                    data += "; ";
                data += info.plainTextLabel() + ": " + typeInfo.toDisplayString(prop);
            }
        }
        return data;
    }
    return QVariant();
}

QVariant ModelDataGenerator::otherMutationsHeader()
{
    switch (role)
    {
    case Qt::DisplayRole:
        return QObject::tr("Weitere Befunde");
    case PatientPropertyModel::DataAggregationNatureRole:
        return DataAggregation::Text;
    default:
        return QVariant();
    }
}

QVariant ModelDataGenerator::completenessDatum(CompletenessField value)
{

    if (!hasPathology)
    {
        switch (role)
        {
        case Qt::DisplayRole:
            return QObject::tr("Keine Befunde");
        }
        return QVariant();
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
    case FISHCompleteness:
        result = checker.isFishComplete(&missingProperties);
        break;
    default:
        return QVariant();
    }

    switch (role)
    {
    case Qt::DisplayRole:
    {
        switch (result)
        {
        case ResultCompletenessChecker::Complete:
            return QObject::tr("ja");
        case ResultCompletenessChecker::Incomplete:
        case ResultCompletenessChecker::PartialResult:
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
        if (result == ResultCompletenessChecker::Incomplete)
        {
            missing += (missingProperties.size() > 1) ? QObject::tr(" fehlen") : QObject::tr(" fehlt");
        }
        else
        {
            missing.prepend("Nachzuscoren: ");
        }
        return missing;
    }
    case PatientModel::VariantDataRole:
    {
        switch (result)
        {
        case ResultCompletenessChecker::Complete:
            return true;
        case ResultCompletenessChecker::Incomplete:
        case ResultCompletenessChecker::PartialResult:
            return false;
        case ResultCompletenessChecker::Absent:
            return false;
        case ResultCompletenessChecker::Undefined:
        case ResultCompletenessChecker::OnlyOptional:
            return true;
        }
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
    Q_UNUSED(value)
    switch (role)
    {
    case Qt::DisplayRole:
        switch (field)
        {
        case IHCCompleteness:
            return QObject::tr("IHC vorliegend");
        case MutationCompleteness:
            return QObject::tr("Mut. vorliegend");
        case FISHCompleteness:
            return QObject::tr("FISH vorliegend");
        }
    case PatientPropertyModel::DataAggregationNatureRole:
        return DataAggregation::Boolean;
    default:
        return QVariant();
    }
}

QVariant ModelDataGenerator::actionableResultsDatum(ActionableResultsField value)
{
    if (!p->hasDisease())
    {
        return QVariant();
    }
    ActionableResultChecker checker(p);
    QList<PathologyPropertyInfo> actionableResults = checker.actionableResults();
    if (actionableResults.isEmpty())
    {
        switch (role)
        {
        case Qt::DisplayRole:
            return QObject::tr("-");
        case PatientModel::VariantDataRole:
            return 0;
        }
        return QVariant();
    }

    switch (value)
    {
    case ActionableResultFieldNames:
    {
        switch (role)
        {
        case Qt::DisplayRole:
        {
            QString data;
            foreach (const PathologyPropertyInfo& field, actionableResults)
            {
                if (!data.isEmpty())
                    data += ", ";
                data += field.plainTextLabel();
            }
            return data;
        }
        case PatientModel::VariantDataRole:
        {
            QVariantList list;
            foreach (const PathologyPropertyInfo& field, actionableResults)
            {
                list << QVariant::fromValue(field);
            }
            return list;
        }
        }
    }
    case ActionableResultNumber:
    {
        switch (role)
        {
        case Qt::DisplayRole:
        case PatientModel::VariantDataRole:
            return actionableResults.size();
        }
    }
    }

    return QVariant();
}

QVariant ModelDataGenerator::actionableResultsHeader(ActionableResultsField value)
{
    switch (role)
    {
    case Qt::DisplayRole:
        return QObject::tr("Therapierelevant");
    case PatientPropertyModel::DataAggregationNatureRole:
        switch (value)
        {
        case ActionableResultFieldNames:
            return DataAggregation::Boolean;
        case ActionableResultNumber:
            return DataAggregation::NumericSum;
        }
    default:
        return QVariant();
    }
}
