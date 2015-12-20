/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 03.08.2015
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

#ifndef PATIENTPARSEPAGE_H
#define PATIENTPARSEPAGE_H

#include <QWizardPage>

#include "pathologyparser.h"

class QDateEdit;
class PatientEnterForm;
class EntitySelectionWidgetV2;
class PathologyPropertiesTableModel;
class PathologyPropertiesTableFilterModel;
class PathologyPropertiesTableView;
class PathologyPropertyInfo;
class QMenu;

class PatientParsePage : public QWizardPage
{
    Q_OBJECT
public:
    PatientParsePage(const PatientParseResults& results);

    void saveData();
    virtual bool isComplete() const;

protected slots:

    void showFullText();
    void addPropertyTriggered(QAction* action);

protected:

    QMenu* buildPropertyMenu();
    void fillPropertySubmenu(QMenu* menu, const QList<PathologyPropertyInfo>& infos);

    PatientParseResults                     results;
    PatientEnterForm*                       patientEnterForm;
    EntitySelectionWidgetV2*                entitySelectionWidget;
    QDateEdit*                              initialDiagnosisEdit;
    PathologyPropertiesTableModel*          model;
    PathologyPropertiesTableFilterModel *   filterModel;
    PathologyPropertiesTableView*           view;
};

#endif // PATIENTPARSEPAGE_H
