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

// Local includes

#include "patientmanager.h"

namespace
{
    static const int RowCount = 3;

    enum ColumnValues
    {
        Surname     = 0,
        FirstName   = 1,
        DateOfBirth = 2
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
    connect(PatientManager::instance(), SIGNAL(patientDataChanged(Patient::Ptr)),
            this, SLOT(patientDataChanged(Patient::Ptr)));
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
        switch (index.column())
        {
        case Surname:
            return p->surname;
        case FirstName:
            return p->firstName;
        case DateOfBirth:
            return p->dateOfBirth.toString(tr("dd.MM.yyyy"));
        }
        break;
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
    return RowCount;
}

Qt::ItemFlags PatientModel::flags(const QModelIndex& index) const
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
    if (parent.isValid() || column < 0 || column >= RowCount || row < 0)
    {
        return QModelIndex();
    }
    if (row >= PatientManager::instance()->numberOfPatients())
    {
        return QModelIndex();
    }
    return createIndex(row, column, PatientManager::instance()->patientId(row));
}

QModelIndex PatientModel::parent(const QModelIndex& index) const
{
    return QModelIndex();
}


QModelIndex PatientModel::indexForPatient(const Patient::Ptr& patient)
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

Patient::Ptr PatientModel::patientForIndex(const QModelIndex& index)
{
    return PatientManager::instance()->patient(index.row());
}

void PatientModel::patientAdded(int index, const Patient::Ptr& patient)
{
    beginInsertRows(QModelIndex(), index, index);
    endInsertRows();
}

void PatientModel::patientDataChanged(const Patient::Ptr& patient)
{
    QModelIndex index = indexForPatient(patient);
    emit dataChanged(index, index);
}

void PatientModel::patientAboutToBeRemoved(int index, const Patient::Ptr& patient)
{
    beginRemoveRows(QModelIndex(), index, index);
}

void PatientModel::patientRemoved(const Patient::Ptr& patient)
{
    endRemoveRows();
}

