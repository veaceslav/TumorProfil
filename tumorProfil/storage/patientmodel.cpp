/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 06.02.2012
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

#include "patientmodel.h"

// Qt includes

#include <QDebug>

// Local includes

#include "patientmanager.h"
#include "pathologypropertyinfo.h"

namespace
{
    enum ColumnValues
    {
        Surname        = 0,
        FirstName      = 1,
        DateOfBirth    = 2,
        HasTumorprofil = 3,
        Entity         = 4,
        ColumnCount
    };
}

PatientModel::PatientModel(QObject *parent) :
    QAbstractItemModel(parent)
{
    connect(PatientManager::instance(), SIGNAL(patientAdded(int,Patient::Ptr)),
            this, SLOT(patientAdded(int,Patient::Ptr)));
    connect(PatientManager::instance(), SIGNAL(patientRemoved(Patient::Ptr)),
            this, SLOT(patientRemoved(Patient::Ptr)));
    connect(PatientManager::instance(), SIGNAL(patientAboutToBeRemoved(int,Patient::Ptr)),
            this, SLOT(patientAboutToBeRemoved(int,Patient::Ptr)));
    connect(PatientManager::instance(), SIGNAL(patientDataChanged(Patient::Ptr, int)),
            this, SLOT(patientDataChanged(Patient::Ptr, int)));
}

static bool hasTumorprofil(const Patient::Ptr& p)
{
    return p->hasDisease() && p->firstDisease().hasPathology(PathologyContextInfo::Tumorprofil);
}

void PatientModel::installRoleDataProvider(Qt::ItemDataRole role, RoleDataProvider* provider)
{
    m_roleDataProviders[role] = provider;
}

QVariant PatientModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    Patient::Ptr p = PatientManager::instance()->patient(index.row());
    switch (role)
    {
    case Qt::DisplayRole:
    case VariantDataRole:
        switch (index.column())
        {
        case Surname:
            return p->surname;
        case FirstName:
            return p->firstName;
        case DateOfBirth:
            return (role == Qt::DisplayRole) ?
                        QVariant(p->dateOfBirth.toString(tr("dd.MM.yyyy"))) :
                        QVariant(p->dateOfBirth);
        case HasTumorprofil:
            if (role == Qt::DisplayRole)
            {
                return hasTumorprofil(p) ? "*" : QString();
            }
            else
            {
                return hasTumorprofil(p);
            }
            break;
        case Entity:
        {
            if (p->hasDisease())
            {
                const Disease& disease = p->firstDisease();
                if (disease.hasPathology())
                {
                    if (role == VariantDataRole)
                    {
                        return disease.entity();
                    }
                    switch (disease.entity())
                    {
                    case Pathology::PulmonaryAdeno:
                    case Pathology::PulmonaryBronchoalveloar:
                        return "NSCLC/A";
                    case Pathology::PulmonaryLargeCell:
                        return "NSCLC/LC";
                    case Pathology::PulmonarySquamous:
                        return "NSCLC/P";
                    case Pathology::PulmonaryAdenosquamous:
                        return "NSCLC/P";
                    case Pathology::PulmonaryOtherCarcinoma:
                        return "NSCLC/?";
                    case Pathology::ColorectalAdeno:
                        return "CRC";
                    case Pathology::Cholangiocarcinoma:
                        return "CCC";
                    case Pathology::RenalCell:
                        return "RCC";
                    case Pathology::Esophageal:
                        return "Öso";
                    case Pathology::EsophagogastrealJunction:
                        return "AEG";
                    case Pathology::Gastric:
                        return "Magen";
                    case Pathology::Breast:
                        return "Mamma";
                    case Pathology::TransitionalCell:
                        return "Urothel";
                    case Pathology::Thyroid:
                        return QObject::tr("Schilddr.");
                    case Pathology::Melanoma:
                        return QObject::tr("Melanom");
                    case Pathology::UnknownEntity:
                        return "?";
                    }
                }
            }
        }
        }
        break;
    case PatientPtrRole:
        return QVariant::fromValue(p);
    case HasTumorprofilRole:
        return hasTumorprofil(p);
    }
    RoleDataProvider* provider = m_roleDataProviders.value(role);
    if (provider)
    {
        return provider->data(this, index, p);
    }
    return QVariant();
}

QVariant PatientModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation ==Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case Surname:
            return tr("Nachname");
        case FirstName:
            return tr("Vorname");
        case DateOfBirth:
            return tr("Geburtsdatum");
        case HasTumorprofil:
        case Entity:
            return QString();
        }
    }
    return QVariant();
}

int PatientModel::rowCount(const QModelIndex& parent ) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return PatientManager::instance()->numberOfPatients();
}

int PatientModel::columnCount(const QModelIndex& parent ) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return ColumnCount;
}

Qt::ItemFlags PatientModel::flags(const QModelIndex&) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

bool PatientModel::hasChildren(const QModelIndex& parent ) const
{
    if (parent.isValid())
    {
        return false;
    }
    return true;
}

QModelIndex PatientModel::index(int row, int column, const QModelIndex& parent ) const
{
    if (parent.isValid() || column < 0 || column >= ColumnCount || row < 0)
    {
        return QModelIndex();
    }
    if (row >= PatientManager::instance()->numberOfPatients())
    {
        return QModelIndex();
    }
    return createIndexForRow(row, column);
}

QModelIndex PatientModel::createIndexForRow(int row, int column) const
{
    return createIndex(row, column, PatientManager::instance()->patientId(row));
}

QModelIndex PatientModel::parent(const QModelIndex&) const
{
    return QModelIndex();
}


QModelIndex PatientModel::indexForPatient(const Patient::Ptr& patient) const
{
    if (!patient)
    {
        return QModelIndex();
    }
    int index = PatientManager::instance()->indexOfPatient(patient);
    if (index == -1)
    {
        return QModelIndex();
    }
    return createIndex(index, 0, patient->id);
}

Patient::Ptr PatientModel::patientForIndex(const QModelIndex& index) const
{
    return PatientManager::instance()->patient(index.row());
}

Patient::Ptr PatientModel::retrievePatient(const QModelIndex& index)
{
    QVariant data = index.data(PatientPtrRole);
    if (!data.isValid())
    {
        return Patient::Ptr();
    }
    return data.value<Patient::Ptr>();
}

void PatientModel::patientAdded(int index, const Patient::Ptr&)
{
    beginInsertRows(QModelIndex(), index, index);
    endInsertRows();
}

void PatientModel::patientDataChanged(const Patient::Ptr& patient, int)
{
    QModelIndex index = indexForPatient(patient);
    emit dataChanged(index, index);
}

void PatientModel::patientAboutToBeRemoved(int index, const Patient::Ptr&)
{
    beginRemoveRows(QModelIndex(), index, index);
}

void PatientModel::patientRemoved(const Patient::Ptr&)
{
    endRemoveRows();
}

