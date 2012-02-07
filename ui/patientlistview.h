/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 30.01.2012
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

#ifndef PATIENTLISTVIEW_H
#define PATIENTLISTVIEW_H

// Qt includes

#include <QTreeView>

// Local includes

#include "patient.h"

class PatientListView : public QTreeView
{
    Q_OBJECT
public:
    explicit PatientListView(QWidget *parent = 0);

signals:

    void activated(const Patient::Ptr& p);

public slots:

    void setCurrentPatient(const Patient::Ptr& p);

protected slots:

    void slotActivated(const QModelIndex &index);

private:

    class PatientListViewPriv;
    PatientListViewPriv* const d;
};

#endif // PATIENTLISTVIEW_H
