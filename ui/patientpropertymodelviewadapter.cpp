/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 28.02.2013
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

#include "patientpropertymodelviewadapter.h"

// Qt includes

#include <QDebug>

// Local includes

#include "pathologypropertyinfo.h"
#include "patientpropertymodel.h"
#include "patientpropertyfiltermodel.h"

class PatientPropertyModelViewAdapter::Private
{
public:
    Private()
        : reportType(InvalidReport)
    {
    }

    PatientPropertyModel* model;
    PatientPropertyFilterModel* filterModel;
    ReportType reportType;

    void adjustModels();
};

PatientPropertyModelViewAdapter::PatientPropertyModelViewAdapter(QObject* parent)
    : QObject(parent),
      d(new Private)
{
    d->model = new PatientPropertyModel(this);
    d->filterModel = new PatientPropertyFilterModel(this);

    d->filterModel->setSourceModel(d->model);
    d->filterModel->setDynamicSortFilter(true);
}

PatientPropertyModelViewAdapter::~PatientPropertyModelViewAdapter()
{
    delete d;
}

PatientPropertyModel* PatientPropertyModelViewAdapter::model() const
{
    return d->model;
}

PatientPropertyFilterModel* PatientPropertyModelViewAdapter::filterModel() const
{
    return d->filterModel;
}

PatientPropertyModelViewAdapter::ReportType PatientPropertyModelViewAdapter::reportType() const
{
    return d->reportType;
}

void PatientPropertyModelViewAdapter::setReportType(int type)
{
    if (d->reportType == type)
    {
        return;
    }
    d->reportType = (ReportType)type;

    d->adjustModels();

    emit reportTypeChanged(d->reportType);
    //resizeColumnsToContents();
}

void PatientPropertyModelViewAdapter::addFilter(const PathologyPropertyInfo& info, const QVariant &value)
{
    PatientPropertyFilterSettings settings = d->filterModel->filterSettings();
    settings.pathologyPropertiesAnd[info.id] = value;
    d->filterModel->setFilterSettings(settings);
}

void PatientPropertyModelViewAdapter::clearFilter()
{
    PatientPropertyFilterSettings settings = d->filterModel->filterSettings();
    settings.pathologyPropertiesAnd.clear();
    d->filterModel->setFilterSettings(settings);
}

void PatientPropertyModelViewAdapter::Private::adjustModels()
{
    // Adjust model
    switch (reportType)
    {
    case OverviewReport:
        model->setProfile(PatientPropertyModel::AllPatientsProfile);
        filterModel->setFilterSettings(PatientPropertyFilterSettings());
        break;
    case PulmonaryAdenoIHCMut:
        model->setProfile(PatientPropertyModel::PulmonaryAdenoProfile);
        filterModel->filterByEntity(QList<Pathology::Entity>()
                                       << Pathology::PulmonaryAdeno
                                       << Pathology::PulmonaryBronchoalveloar
                                       << Pathology::PulmonaryAdenosquamous);
        break;
    case PulmonarySquamousIHCMut:
        model->setProfile(PatientPropertyModel::PulmonarySqamousProfile);
        filterModel->filterByEntity(Pathology::PulmonarySquamous);
        break;
    case CRCIHCMut:
        model->setProfile(PatientPropertyModel::CRCProfile);
        filterModel->filterByEntity(Pathology::ColorectalAdeno);
        break;
    case BreastCaIHCMut:
        model->setProfile(PatientPropertyModel::BreastCaProfile);
        filterModel->filterByEntity(Pathology::Breast);
        break;
    case TumorprofilIHCMut:
    {
        model->setProfile(PatientPropertyModel::AllTumorprofilProfile);
        filterModel->filterByEntity(QList<Pathology::Entity>()
                                       << Pathology::PulmonaryAdeno
                                       << Pathology::PulmonaryBronchoalveloar
                                       << Pathology::PulmonarySquamous
                                       << Pathology::PulmonaryAdenosquamous
                                       << Pathology::ColorectalAdeno
                                       << Pathology::Breast);
        break;
    }
    case EGFRMutation:
    {
        model->setProfile(PatientPropertyModel::EGFRProfile);
        QMap<QString, QVariant> filter;
        filter[PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_EGFR_19_21).id] = true;
        filter[PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_EGFR_18_20).id] = true;
        filterModel->filterByPathologyProperty(filter);
    }
        break;
    case PIK3Mutation:
        model->setProfile(PatientPropertyModel::PIK3Profile);
        filterModel->filterByPathologyProperty(
                    PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_PIK3CA_10_21).id, true);
        break;
    case BRAFMutation:
    {
        model->setProfile(PatientPropertyModel::PIK3Profile);
        QMap<QString, QVariant> filter;
        filter[PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_BRAF_15).id] = true;
        filter[PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_BRAF_11).id] = true;
        filterModel->filterByPathologyProperty(filter);
        break;
    }
    case PTENLoss:
        model->setProfile(PatientPropertyModel::PTENLossProfile);
        filterModel->filterByPathologyProperty(
                    PathologyPropertyInfo::info(PathologyPropertyInfo::IHC_PTEN).id, 0);
        break;
    case NSCLCKRASMutation:
    {
        model->setProfile(PatientPropertyModel::PulmonaryAdenoProfile);
        PatientPropertyFilterSettings settings = filterModel->filterSettings();
        settings.entities = (QList<Pathology::Entity>()
                << Pathology::PulmonaryAdeno
                << Pathology::PulmonaryBronchoalveloar
                << Pathology::PulmonaryAdenosquamous);
        settings.pathologyProperties.clear();
        settings.pathologyProperties[PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_KRAS_2).id]
                = true;
        settings.pathologyProperties[PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_KRAS_3).id]
                = true;
        settings.pathologyProperties[PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_NRAS_2_4).id]
                = true;
        filterModel->setFilterSettings(settings);
        break;
    }
    case NSCLCHer2Amplification:
    {
        model->setProfile(PatientPropertyModel::PulmonaryAdenoProfile);
        PatientPropertyFilterSettings settings = filterModel->filterSettings();
        settings.entities = QList<Pathology::Entity>()
                << Pathology::PulmonaryAdeno
                << Pathology::PulmonaryBronchoalveloar
                << Pathology::PulmonaryAdenosquamous;
        settings.pathologyProperties.clear();
        settings.pathologyProperties[PathologyPropertyInfo::info(PathologyPropertyInfo::Comb_HER2).id]
                = true;
        filterModel->setFilterSettings(settings);
        break;
    }
    case ALKAmplification:
        model->setProfile(PatientPropertyModel::PulmonaryAdenoProfile);
        filterModel->filterByPathologyProperty(
                    PathologyPropertyInfo::info(PathologyPropertyInfo::Fish_ALK).id, true);
        break;
    case cMetOverexpression:
        model->setProfile(PatientPropertyModel::AllTumorprofilProfile);
        filterModel->filterByPathologyProperty(
                    PathologyPropertyInfo::info(PathologyPropertyInfo::Comb_cMetActivation).id, true);
    case InvalidReport:
        break;
    }
}
