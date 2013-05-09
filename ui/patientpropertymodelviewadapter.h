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

#ifndef PATIENTPROPERTYMODELVIEWADAPTER_H
#define PATIENTPROPERTYMODELVIEWADAPTER_H

// Qt includes

#include <QObject>

class PatientPropertyModel;
class PatientPropertyFilterModel;
class PathologyPropertyInfo;

class PatientPropertyModelViewAdapter : public QObject
{
    Q_OBJECT

public:

    enum ReportType
    {
        InvalidReport,
        OverviewReport,
        PulmonaryAdenoIHCMut,
        PulmonarySquamousIHCMut,
        CRCIHCMut,
        BreastCaIHCMut,
        TumorprofilIHCMut,
        PIK3Mutation,
        EGFRMutation,
        BRAFMutation,
        PTENLoss,
        NSCLCKRASMutation,
        NSCLCHer2Amplification,
        ALKAmplification
    };

    PatientPropertyModelViewAdapter(QObject* parent = 0);
    ~PatientPropertyModelViewAdapter();

    PatientPropertyModel* model() const;
    PatientPropertyFilterModel* filterModel() const;

    void setReportType(int type);
    ReportType reportType() const;

Q_SIGNALS:

    void reportTypeChanged(ReportType type);

public Q_SLOTS:

    void addFilter(const PathologyPropertyInfo& info, const QVariant& value);
    void clearFilter();

private:

    class Private;
    Private* const d;
};

#endif // PATIENTPROPERTYMODELVIEWADAPTER_H
