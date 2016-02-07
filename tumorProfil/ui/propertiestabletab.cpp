/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 01.08.2015
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

#include "propertiestabletab.h"

#include <QGridLayout>
#include <QPushButton>

#include "entityselectionwidgetv2.h"
#include "pathologypropertiestableview.h"
#include "patient.h"
#include "patientmanager.h"

class PropertiesTableTab::PropertiesTableTabPriv
{
public:
    PropertiesTableTabPriv()
        : diseaseIndex(-1),
          editingEnabled(false),
          edited(false),
          entitySelectionWidget(0),
          manualAddButton(0),
          model(0),
          filterModel(0),
          view(0)
    {
    }

    Patient::Ptr                            currentPatient;
    int                                     diseaseIndex;

    bool                                    editingEnabled;
    bool                                    edited;

    EntitySelectionWidgetV2*                entitySelectionWidget;
    QPushButton*                            manualAddButton;
    PathologyPropertiesTableModel*          model;
    PathologyPropertiesTableFilterModel *   filterModel;
    PathologyPropertiesTableView*           view;

    Disease& currentDisease()
    {
        return currentPatient->diseases[diseaseIndex];
    }
    bool hasValidDisease() const
    {
        return diseaseIndex >= 0;
    }
};

PropertiesTableTab::PropertiesTableTab(QWidget *parent)
    : QWidget(parent),
      d(new PropertiesTableTabPriv)
{
    QGridLayout* layout = new QGridLayout;

    d->entitySelectionWidget = new EntitySelectionWidgetV2;
    layout->addWidget(d->entitySelectionWidget, 0, 0);

    d->manualAddButton = new QPushButton(tr("Hinzufügen..."));
    layout->addWidget(d->manualAddButton);

    d->model = new PathologyPropertiesTableModel(this);
    d->filterModel = new PathologyPropertiesTableFilterModel(this);
    d->view  = new PathologyPropertiesTableView;
    d->view->setModels(d->model, d->filterModel);
    layout->addWidget(d->view, 1, 0, 1, 2);
    layout->setRowStretch(1, 1);

    setLayout(layout);

    connect(d->entitySelectionWidget, &EntitySelectionWidgetV2::entityChanged, this, &PropertiesTableTab::entityChanged);
    connect(d->model, &PathologyPropertiesTableModel::propertyEdited, this, &PropertiesTableTab::propertyEdited);
}

PropertiesTableTab::~PropertiesTableTab()
{
    delete d;
}

void PropertiesTableTab::setEditingEnabled(bool enabled)
{
    d->editingEnabled = enabled;
    d->model->setEditingEnabled(enabled);
}

QString PropertiesTableTab::tabLabel() const
{
    return tr("Befunde");
}

void PropertiesTableTab::setDisease(const Patient::Ptr& p, int diseaseIndex)
{
    if (d->currentPatient == p && d->diseaseIndex == diseaseIndex)
    {
        return;
    }

    if (d->currentPatient)
    {
        save();
    }

    d->currentPatient = p;
    d->diseaseIndex   = diseaseIndex;

    setEnabled(p);
    d->edited = false;
    if (!p)
    {
        d->diseaseIndex = -1;
        return;
    }

    if (d->hasValidDisease())
    {
        d->entitySelectionWidget->setEntity(d->currentDisease().entity());
        d->entitySelectionWidget->setMode(EntitySelectionWidgetV2::DisplayMode);
        d->model->setPathologies(d->currentDisease().pathologies);
    }
    else
    {
        d->entitySelectionWidget->setMode(EntitySelectionWidgetV2::EditMode);
        // Enable when an entity was selected (see entityChanged)
        d->view->setEnabled(false);
    }
}

void PropertiesTableTab::save()
{
    if (!d->hasValidDisease() || !d->editingEnabled || !d->edited)
    {
        return;
    }

    d->currentDisease().pathologies = d->model->pathologiesConsolidated();
    for (int i=0; i<d->currentDisease().pathologies.size(); ++i)
    {
        d->currentDisease().pathologies[i].entity = d->entitySelectionWidget->entity();
    }

    PatientManager::instance()->updateData(d->currentPatient,
                                           PatientManager::ChangedPathologyData |
                                           PatientManager::ChangedDiseaseMetadata |
                                           PatientManager::ChangedPatientProperties |
                                           PatientManager::ChangedDiseaseProperties);
}

void PropertiesTableTab::entityChanged(Pathology::Entity entity)
{
    if (entity != Pathology::UnknownEntity)
    {
        if (!d->hasValidDisease())
        {
            d->currentPatient->diseases << Disease();
            d->diseaseIndex = 0;
            d->view->setEnabled(true);
        }
    }
    d->edited = true;
}

void PropertiesTableTab::propertyEdited()
{
    d->edited = true;
}
