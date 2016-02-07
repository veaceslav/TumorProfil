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

#ifndef PATIENTPROPERTYMODEL_H
#define PATIENTPROPERTYMODEL_H

#include "patientmodel.h"

class PatientPropertyModel : public PatientModel
{
    Q_OBJECT
public:
    explicit PatientPropertyModel(QObject *parent = 0);
    ~PatientPropertyModel();

    enum Profile
    {
        AllPatientsProfile,
        PulmonaryAdenoProfile,
        PulmonarySqamousProfile,
        CRCProfile,
        BreastCaProfile,
        AllTumorprofilProfile,
        EGFRProfile,
        PIK3Profile,
        PTENLossProfile
    };

    enum Roles
    {
        // for headerData: returns the PathologyPropertyInfo if the section represents a property
        PathologyPropertyInfoRole = Qt::UserRole + 100,
        // for data: returns the Property from Pathology's properties
        PathologyPropertyRole     = Qt::UserRole + 101,
        // returns if a datum is text, numeric, etc.
        DataAggregationNatureRole = Qt::UserRole + 102
    };

    void setProfile(Profile profile);
    Profile profile() const;
    
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent ) const;

signals:
    
public slots:
    
private:

    QVariant overviewData(const Patient::Ptr& p, int field, int role) const;
    QVariant fieldData(const Patient::Ptr& p, int field, int role) const;
    QVariant completenessData(const Patient::Ptr& p, int field, int role) const;

    class PatientPropertyModelPriv;
    PatientPropertyModelPriv* const d;
};

#endif // PATIENTPROPERTYMODEL_H
