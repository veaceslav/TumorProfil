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

#ifndef PATIENTMODEL_H
#define PATIENTMODEL_H

// Qt includes

#include <QAbstractItemModel>

// Local includes

#include "patient.h"

class PatientModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit PatientModel(QObject *parent = 0);

    QModelIndex indexForPatient(const Patient::Ptr& patient);
    Patient::Ptr patientForIndex(const QModelIndex& index);

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex& index) const;

protected slots:

    void patientAdded(int index, const Patient::Ptr& patient);
    void patientDataChanged(const Patient::Ptr& patient);
    void patientAboutToBeRemoved(int index, const Patient::Ptr& patient);
    void patientRemoved(const Patient::Ptr& patient);

};

#endif // PATIENTMODEL_H
