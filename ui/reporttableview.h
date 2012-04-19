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

#ifndef REPORTTABLEVIEW_H
#define REPORTTABLEVIEW_H

#include <QTableView>

#include "patient.h"

class PatientPropertyModel;
class PatientPropertyFilterModel;

class ReportTableView : public QTableView
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
        PIK3Mutation,
        EGFRMutation,
        PTENLoss
    };

    explicit ReportTableView(QWidget *parent = 0);
    ~ReportTableView();
    
    ReportType reportType() const;
    PatientPropertyModel          *model() const;
    PatientPropertyFilterModel    *filterModel() const;

signals:

    void activated(const Patient::Ptr& p);

public slots:

    void setReportType(int type);
    void copy();

protected:

    void keyPressEvent(QKeyEvent *event);

protected slots:

    void indexActivated(const QModelIndex& index);
    
private:

    class ReportTableViewPriv;
    ReportTableViewPriv* const d;
};

#endif // REPORTTABLEVIEW_H
