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

#include <QDebug>

// Local includes

#include "dataaggregator.h"
#include "modeldatagenerator.h"
#include "patientpropertymodel.h"
#include "pathologypropertyinfo.h"

class PatientPropertyModel::PatientPropertyModelPriv
{
public:
    PatientPropertyModelPriv()
    {
    }

    PatientPropertyModel::Profile profile;
    QList<PathologyPropertyInfo>  infos;

    void setInfos()
    {
        infos.clear();
        QList<PathologyPropertyInfo::Property> props;
        bool addMutations = false;
        switch (profile)
        {
        case AllPatientsProfile:
            break;
        case PatientPropertyModel::PulmonaryAdenoProfile:
            props << PathologyPropertyInfo::IHC_PTEN
                  << PathologyPropertyInfo::IHC_pAKT
                  << PathologyPropertyInfo::IHC_pERK
                  << PathologyPropertyInfo::IHC_ALK
                  << PathologyPropertyInfo::IHC_ROS1
                  << PathologyPropertyInfo::Comb_cMetActivation
                  << PathologyPropertyInfo::Comb_cMetIHC3plusScore
                  //<< PathologyPropertyInfo::IHC_HER2
                  //<< PathologyPropertyInfo::IHC_HER2_DAKO
                  << PathologyPropertyInfo::Fish_HER2
                  << PathologyPropertyInfo::Comb_HER2
                  << PathologyPropertyInfo::IHC_cMET
                  << PathologyPropertyInfo::Fish_ALK
                  << PathologyPropertyInfo::Fish_ROS1;
            addMutations = true;
            break;
        case PatientPropertyModel::PulmonarySqamousProfile:
            props << PathologyPropertyInfo::IHC_PTEN
                  << PathologyPropertyInfo::IHC_pAKT
                  << PathologyPropertyInfo::IHC_pERK
                  << PathologyPropertyInfo::Fish_FGFR1
                  << PathologyPropertyInfo::Fish_PIK3CA;
            addMutations = true;
            break;
        case PatientPropertyModel::CRCProfile:
            props << PathologyPropertyInfo::IHC_PTEN
                  << PathologyPropertyInfo::IHC_pAKT
                  << PathologyPropertyInfo::IHC_pERK
                  << PathologyPropertyInfo::IHC_pP70S6K
                  << PathologyPropertyInfo::Comb_cMetActivation;
            addMutations = true;
            break;
        case PatientPropertyModel::BreastCaProfile:
            props << PathologyPropertyInfo::IHC_PTEN
                  << PathologyPropertyInfo::IHC_pAKT
                  << PathologyPropertyInfo::IHC_pP70S6K
                  << PathologyPropertyInfo::IHC_pERK
                  << PathologyPropertyInfo::Fish_PIK3CA
                  << PathologyPropertyInfo::Fish_FGFR1
                  << PathologyPropertyInfo::IHC_ER
                  << PathologyPropertyInfo::IHC_PR
                  << PathologyPropertyInfo::IHC_HER2_DAKO
                  << PathologyPropertyInfo::Fish_HER2
                  << PathologyPropertyInfo::Comb_HER2
                  << PathologyPropertyInfo::Comb_HormoneReceptor
                  << PathologyPropertyInfo::Comb_TripleNegative;
            addMutations = true;
            break;
        case PatientPropertyModel::AllTumorprofilProfile:
            props << PathologyPropertyInfo::IHC_PTEN
                  << PathologyPropertyInfo::IHC_pAKT
                  << PathologyPropertyInfo::IHC_pERK
                  << PathologyPropertyInfo::IHC_ALK
                  << PathologyPropertyInfo::IHC_HER2_DAKO
                  << PathologyPropertyInfo::IHC_pP70S6K
                  << PathologyPropertyInfo::Fish_FGFR1;
            addMutations = true;
            break;
        case PatientPropertyModel::EGFRProfile:
            props << PathologyPropertyInfo::Mut_EGFR_19_21
                  << PathologyPropertyInfo::Mut_EGFR_18_20
                  << PathologyPropertyInfo::Mut_KRAS_2
                  << PathologyPropertyInfo::Mut_KRAS_3
                  << PathologyPropertyInfo::Mut_PIK3CA_10_21;
            break;
       case PatientPropertyModel::PIK3Profile:
            props << PathologyPropertyInfo::Mut_PIK3CA_10_21
                  << PathologyPropertyInfo::Mut_KRAS_2
                  << PathologyPropertyInfo::Mut_EGFR_19_21;
            break;
        case PatientPropertyModel::PTENLossProfile:
            props << PathologyPropertyInfo::IHC_PTEN;
            break;
        }
        foreach (PathologyPropertyInfo::Property property, props)
        {
            infos << PathologyPropertyInfo::info(property);
        }
        if (addMutations)
        {
            infos += PathologyPropertyInfo::allMutations();
        }
    }
};

PatientPropertyModel::PatientPropertyModel(QObject *parent) :
    PatientModel(parent),
    d(new PatientPropertyModelPriv)
{
}

PatientPropertyModel::~PatientPropertyModel()
{
    delete d;
}

void PatientPropertyModel::setProfile(Profile profile)
{
    if (d->profile == profile)
    {
        return;
    }
    beginResetModel();
    d->profile = profile;
    d->setInfos();
    endResetModel();
}

PatientPropertyModel::Profile PatientPropertyModel::profile() const
{
    return d->profile;
}

namespace
{
enum Columns
{
    // first three columns are handled by PatientModel
    PatientModelColumns = 3,

    // For overview mode
    OverviewEntityColumn = 0,
    OverviewResultsColum = 1,
    OverviewColumnCount  = 2,

    // For per-mutation mode
    EntityNameColumnCount = 1,

    // for other mutations column
    OtherMutationsColumnCount = 1,

    // for completeness columns
    IHCCompletenessColumn      = 0,
    MutationCompletenessColumn = 1,
    FISHCompletenessColumn     = 2,
    CompletenessColumnCount    = 3,

    // for actionable results
    ActionableResultsColumnCount = 2
};
}

class DataGenerator : public ModelDataGenerator
{
public:

    DataGenerator(PatientPropertyModel::Profile profile,
                  const QList<PathologyPropertyInfo>&  infos,
                  const Patient::Ptr& p, int column, int role)
        : ModelDataGenerator(infos, p, role, column - PatientModelColumns),
          profile(profile),
          withAdditionalInfo(true)
    {
    }

    PatientPropertyModel::Profile profile;
    bool withAdditionalInfo;

    QVariant overviewData()
    {
        if (!hasPathology)
        {
            return QVariant();
        }

        switch (role)
        {
        case Qt::DisplayRole:
        case PatientPropertyModel::VariantDataRole:
            switch (field)
            {
            case OverviewEntityColumn:
                return entityNameDatum();
            case OverviewResultsColum:
                int sum = 0;
                foreach (const Pathology& path, p->firstDisease().pathologies)
                {
                    sum += path.properties.size();
                }
                return sum;
            }
            return QString();
        }
        return QVariant();
    }

    QVariant overviewHeader()
    {
        switch (role)
        {
        case Qt::DisplayRole:
            switch (field)
            {
            case OverviewEntityColumn:
                return QObject::tr("Entität");
            case OverviewResultsColum:
                return QObject::tr("Anzahl Befunde");
            }
            return QString();
        case PatientPropertyModel::DataAggregationNatureRole:
            switch (field)
            {
            case OverviewResultsColum:
                return DataAggregation::NumericSum;
            default:
                break;
            }
        default:
            break;
        }
        return QVariant();
    }

    int overviewColumnCount()
    {
        return OverviewColumnCount;
    }

    QVariant profileData()
    {
        if (withAdditionalInfo && field < PatientAdditionalInfoColumnCount)
        {
            return additionalInfoDatum();
        }
        field -= PatientAdditionalInfoColumnCount;
        if (field < infos.size())
        {
            return fieldDatum();
        }
        field -= infos.size();
        if (field < OtherMutationsColumnCount)
        {
            return otherMutationsDatum();
        }
        field -= OtherMutationsColumnCount;

        if (field < CompletenessColumnCount)
        {
            // trans-enum casting
            CompletenessField comp = static_cast<CompletenessField>(field);
            return completenessDatum(comp);
        }
        field -= CompletenessColumnCount;

        if (field < ActionableResultsColumnCount)
        {
            return actionableResultsDatum((ActionableResultsField)field);
        }
        field -= ActionableResultsColumnCount;
        return QVariant();
    }

    QVariant profileHeader()
    {
        if (withAdditionalInfo && field < PatientAdditionalInfoColumnCount)
        {
            return additionalInfoHeader();
        }
        field -= PatientAdditionalInfoColumnCount;
        if (field < infos.size())
        {
            return fieldHeader();
        }
        field -= infos.size();
        if (field < OtherMutationsColumnCount)
        {
            return otherMutationsHeader();
        }
        field -= OtherMutationsColumnCount;
        if (field < CompletenessColumnCount)
        {
            // trans-enum casting
            CompletenessField comp = static_cast<CompletenessField>(field);
            return completenessHeader(comp);
        }
        field -= CompletenessColumnCount;
        if (field < ActionableResultsColumnCount)
        {
            return actionableResultsHeader((ActionableResultsField)field);
        }
        field -= ActionableResultsColumnCount;
        return QVariant();
    }

    int profileColumnCount()
    {
        return (withAdditionalInfo ? PatientAdditionalInfoColumnCount : 0) +
                infos.size() + OtherMutationsColumnCount +
                CompletenessColumnCount + ActionableResultsColumnCount;
    }

    QVariant mutationOverviewData()
    {
        if (field < EntityNameColumnCount)
        {
            return entityNameDatum();
        }
        field -= EntityNameColumnCount;
        if (field < infos.size())
        {
            return fieldDatum();
        }
        field -= infos.size();
        if (field < OtherMutationsColumnCount)
        {
            return otherMutationsDatum();
        }
        return QVariant();
    }

    QVariant mutationOverviewHeader()
    {
        if (field < EntityNameColumnCount)
        {
            return overviewHeader();
        }
        field -= EntityNameColumnCount;
        if (field < infos.size())
        {
            return fieldHeader();
        }
        field -= infos.size();
        if (field < OtherMutationsColumnCount)
        {
            return otherMutationsHeader();
        }
        return QVariant();
    }

    int mutationOverviewColumnCount()
    {
        return EntityNameColumnCount + infos.size() + OtherMutationsColumnCount;
    }
};

QVariant PatientPropertyModel::data(const QModelIndex& index, int role) const
{
    if (index.column() < PatientModelColumns || role == PatientPtrRole || role == HasTumorprofilRole)
    {
        return PatientModel::data(index, role);
    }

    Patient::Ptr p = patientForIndex(index);
    if (!p)
    {
        return QVariant();
    }

    DataGenerator generator(d->profile, d->infos, p, index.column(), role);

    switch (d->profile)
    {
    case AllPatientsProfile:
        return generator.overviewData();
    case PIK3Profile:
    case PTENLossProfile:
        return generator.mutationOverviewData();
    default: // Standard profiles
        return generator.profileData();
    }
    return QVariant();
}

QVariant PatientPropertyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical)
    {
        return QVariant();
    }

    if (section < PatientModelColumns)
    {
        return PatientModel::headerData(section, orientation, role);
    }

    DataGenerator generator(d->profile, d->infos, Patient::Ptr(), section, role);

    switch (d->profile)
    {
    case AllPatientsProfile:
        return generator.overviewHeader();
    case PIK3Profile:
    case PTENLossProfile:
        return generator.mutationOverviewHeader();
    default: // Standard profiles
        return generator.profileHeader();
    }
    return QVariant();
}

int PatientPropertyModel::columnCount(const QModelIndex&) const
{
    DataGenerator generator(d->profile, d->infos, Patient::Ptr(), 0, 0);

    switch (d->profile)
    {
    case AllPatientsProfile:
        return PatientModelColumns + generator.overviewColumnCount();
    case PIK3Profile:
    case PTENLossProfile:
        return PatientModelColumns + generator.mutationOverviewColumnCount();
    default: // Standard profiles
        return PatientModelColumns + generator.profileColumnCount();
    }
    return 0;
}

QModelIndex PatientPropertyModel::index(int row, int column, const QModelIndex& parent ) const
{
    if (column < PatientModelColumns)
    {
        return PatientModel::index(row, column, parent);
    }

    if (parent.isValid() || column < 0 || row < 0)
    {
        return QModelIndex();
    }
    if (row >= rowCount() || column >= columnCount())
    {
        return QModelIndex();
    }
    return createIndexForRow(row, column);
}


