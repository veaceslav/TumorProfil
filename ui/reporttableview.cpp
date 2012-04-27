/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 23.02.2012
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

#include "reporttableview.h"

// Qt includes

#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QHeaderView>
#include <QKeyEvent>
#include <QKeySequence>

// Local includes

#include "patientpropertymodel.h"
#include "patientpropertyfiltermodel.h"
#include "pathologypropertyinfo.h"

class ReportTableView::ReportTableViewPriv
{
public:
    ReportTableViewPriv()
        : model(0),
          filterModel(0),
          reportType(ReportTableView::InvalidReport)
    {
    }

    PatientPropertyModel          *model;
    PatientPropertyFilterModel    *filterModel;

    ReportTableView::ReportType    reportType;
};

ReportTableView::ReportTableView(QWidget *parent) :
    QTableView(parent),
    d(new ReportTableViewPriv)
{
    verticalHeader()->hide();
    d->model = new PatientPropertyModel(this);
    d->filterModel = new PatientPropertyFilterModel(this);

    d->filterModel->setSourceModel(d->model);
    d->filterModel->setDynamicSortFilter(true);
    setModel(d->filterModel);

    sortByColumn(0, Qt::AscendingOrder);
    setSortingEnabled(true);

    connect(this, SIGNAL(activated(QModelIndex)),
            this, SLOT(indexActivated(QModelIndex)));
}

ReportTableView::~ReportTableView()
{
    delete d;
}

ReportTableView::ReportType ReportTableView::reportType() const
{
    return d->reportType;
}

PatientPropertyModel *ReportTableView::model() const
{
    return d->model;
}

PatientPropertyFilterModel *ReportTableView::filterModel() const
{
    return d->filterModel;
}

void ReportTableView::setReportType(int type)
{
    if (d->reportType == type)
    {
        return;
    }
    d->reportType = (ReportType)type;

    // Adjust model
    switch (d->reportType)
    {
    case OverviewReport:
        d->model->setProfile(PatientPropertyModel::AllPatientsProfile);
        d->filterModel->setFilterSettings(PatientPropertyFilterSettings());
        break;
    case PulmonaryAdenoIHCMut:
        d->model->setProfile(PatientPropertyModel::PulmonaryAdenoProfile);
        d->filterModel->filterByEntity(Pathology::PulmonaryAdeno);
        break;
    case PulmonarySquamousIHCMut:
        d->model->setProfile(PatientPropertyModel::PulmonarySqamousProfile);
        d->filterModel->filterByEntity(Pathology::PulmonarySquamous);
        break;
    case CRCIHCMut:
        d->model->setProfile(PatientPropertyModel::CRCProfile);
        d->filterModel->filterByEntity(Pathology::ColorectalAdeno);
        break;
    case EGFRMutation:
        d->model->setProfile(PatientPropertyModel::EGFRProfile);
        d->filterModel->filterByPathologyProperty(
                    PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_EGFR_19_21).id, true);
        break;
    case PIK3Mutation:
        d->model->setProfile(PatientPropertyModel::PIK3Profile);
        d->filterModel->filterByPathologyProperty(
                    PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_PIK3CA_10_21).id, true);
        break;
    case PTENLoss:
        d->model->setProfile(PatientPropertyModel::PTENLossProfile);
        d->filterModel->filterByPathologyProperty(
                    PathologyPropertyInfo::info(PathologyPropertyInfo::IHC_PTEN).id, 0);
        break;
    case InvalidReport:
        break;
    }

    resizeColumnsToContents();
}

void ReportTableView::keyPressEvent(QKeyEvent *event)
{
    if(event->matches(QKeySequence::Copy) )
    {
      copy();
    }
    else
    {
      QTableView::keyPressEvent(event);
    }
}

void ReportTableView::copy()
{
    if (!selectionModel()->hasSelection())
    {
        return;
    }

    QModelIndexList list = selectionModel()->selectedIndexes();
    qSort(list);

    QString copy;

    QModelIndexList::const_iterator it, next;
    for (it = list.begin(); it != list.end(); ++it)
    {
        copy += it->data().toString();
        next = it+1;

        if (next == list.end() || it->row() != next->row())
        {
            copy += '\n';
        }
        else
        {
            copy += '\t';
        }
    }

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(copy);
}

void ReportTableView::indexActivated(const QModelIndex& index)
{
    Patient::Ptr p = PatientModel::retrievePatient(index);
    if (p)
    {
        emit activated(p);
    }
}
