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

#ifndef DISEASETABWIDGET_H
#define DISEASETABWIDGET_H

#include <QTabWidget>

// Local includes

#include "patient.h"

class PathologyTab;

class DiseaseTabWidget : public QTabWidget
{
    Q_OBJECT
public:

    explicit DiseaseTabWidget(QWidget *parent = 0);
    ~DiseaseTabWidget();

    void setEditingEnabled(bool enabled);

signals:

    void editingFinished();

public slots:

    void setPatient(const Patient::Ptr& p);
    void save(const Patient::Ptr& p);

protected slots:

    void slotEntityChanged(Pathology::Entity);
    void slotInitialDiagnosisDateChanged(const QDate&);
    PathologyTab* addPathologyTab(int context);

protected:

    void updatePathologyTabs();
    void loadPathologyData();
    void changeEntity(Pathology::Entity);
    virtual void keyPressEvent(QKeyEvent* e);
    void addTab(PathologyTab* tab);

private:

    class DiseaseTabWidgetPriv;
    DiseaseTabWidgetPriv* const d;
};

#endif // DISEASETABWIDGET_H
