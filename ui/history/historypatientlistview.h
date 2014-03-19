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

#ifndef HISTORYPATIENTLISTVIEW_H
#define HISTORYPATIENTLISTVIEW_H

#include <QTreeView>

#include "patient.h"

class PatientPropertyModelViewAdapter;

class HistoryPatientListView : public QTreeView
{
    Q_OBJECT
public:
    explicit HistoryPatientListView(QWidget *parent = 0);
    void setAdapter(PatientPropertyModelViewAdapter* adapter);

signals:
    
    void activated(const Patient::Ptr& p);
    void clicked(const Patient::Ptr& p);

protected slots:

    void indexActivated(const QModelIndex& index);
    void indexClicked(const QModelIndex& index);

private:

    class HistoryPatientListViewPriv;
    HistoryPatientListViewPriv* const d;
    
};

#endif // HISTORYPATIENTLISTVIEW_H
