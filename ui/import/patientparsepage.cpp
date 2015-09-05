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

#include "patientparsepage.h"

#include <QDateEdit>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

#include "entityselectionwidgetv2.h"
#include "patiententerform.h"
#include "patientdisplay.h"
#include "patientmanager.h"
#include "pathologypropertyinfo.h"
#include "pathologypropertiestablemodel.h"
#include "pathologypropertiestableview.h"


PatientParsePage::PatientParsePage(const PatientParseResults& results)
    : results(results),
      patientEnterForm(0),
      entitySelectionWidget(0),
      initialDiagnosisEdit(0),
      model(0),
      filterModel(0),
      view(0)
{
    setTitle(tr("Befunde prüfen"));
    setSubTitle(tr("Prüfen Sie die folgenden Befunde und nicht erkannten Textabschnitte"));

    QVBoxLayout* layout = new QVBoxLayout;

    if (results.patient)
    {
        // existing patient
        PatientDisplay* display = new PatientDisplay;
        display->setPatient(results.patientData);
        layout->addWidget(display);
    }
    else
    {
        // new patient
        patientEnterForm = new PatientEnterForm;
        patientEnterForm->setValues(results.patientData);
        layout->addWidget(patientEnterForm);
    }

    entitySelectionWidget = new EntitySelectionWidgetV2;
    layout->addWidget(entitySelectionWidget);
    if (results.patient && results.patient->hasDisease())
    {
        entitySelectionWidget->setEntity(results.patient->firstDisease().entity());
        entitySelectionWidget->setMode(EntitySelectionWidgetV2::DisplayMode);
    }
    else
    {
        entitySelectionWidget->setEntity(results.guessedEntity);
        entitySelectionWidget->setMode(EntitySelectionWidgetV2::EditMode);
    }
    connect(entitySelectionWidget, &EntitySelectionWidgetV2::entityChanged, this, &PatientParsePage::completeChanged);

    if (!results.patient)
    {
        initialDiagnosisEdit = new QDateEdit;
        layout->addWidget(initialDiagnosisEdit);
        QDate date;
        date.setDate(results.resultsDate.year(), results.resultsDate.month(), 1);
        if (date.daysTo(results.resultsDate) < 10)
        {
            date = date.addMonths(-1);
        }
        initialDiagnosisEdit->setDate(date);
    }

    model = new PathologyPropertiesTableModel(this);
    filterModel = new PathologyPropertiesTableFilterModel(this);
    view  = new PathologyPropertiesTableView;
    view->setModels(model, filterModel);
    layout->addWidget(view, 2);

    Pathology path;
    path.properties = results.properties;
    model->setPathology(path);

    QLabel* unknownTextLabel = new QLabel(tr("Nicht erkannte Passagen aus den Befunden:"));
    layout->addWidget(unknownTextLabel);
    QTextEdit* textEdit = new QTextEdit;
    textEdit->setReadOnly(true);
    textEdit->setPlainText(results.unrecognizedText);
    layout->addWidget(textEdit, 1);

    QPushButton* fullTextButton = new QPushButton(tr("Zeige Befundtext"));
    connect(fullTextButton, &QPushButton::clicked, this, &PatientParsePage::showFullText);
    layout->addWidget(fullTextButton);

    setLayout(layout);
}

void PatientParsePage::saveData()
{
    if (!results.patient)
    {
        results.patient = PatientManager::instance()->addPatient(patientEnterForm->currentPatient());
    }
    if (!results.patient->hasDisease())
    {
        // add here support for multiple diseases...
        results.patient->diseases << Disease();
    }

    Disease& disease = results.patient->diseases.first();
    if (initialDiagnosisEdit)
    {
        disease.initialDiagnosis = initialDiagnosisEdit->date();
    }

    for (int i=0; i<disease.pathologies.size(); ++i)
    {
        disease.pathologies[i].entity = entitySelectionWidget->entity();
    }

    Pathology* path;
    if (disease.hasProfilePathology() && disease.firstProfilePathology().date.daysTo(results.resultsDate) < 60)
    {
        path = &disease.firstProfilePathology();
    }
    else
    {
        Pathology newPath;
        newPath.context = PathologyContextInfo(PathologyContextInfo::Tumorprofil).id;
        newPath.date = results.resultsDate;
        disease.pathologies << newPath;
        path = &disease.pathologies.last();
    }

    QList<Pathology> paths = model->pathologiesConsolidated();
    // paths size is 1 in our case
    path->properties.merge(paths.first().properties);
    path->reports += results.textPassages;

    PatientManager::instance()->updateData(results.patient,
                                           PatientManager::ChangedPathologyData |
                                           PatientManager::ChangedDiseaseMetadata |
                                           PatientManager::ChangedPatientProperties |
                                           PatientManager::ChangedDiseaseProperties);
}

bool PatientParsePage::isComplete() const
{
    if (!results.patient)
    {
        return entitySelectionWidget->entity() != Pathology::UnknownEntity;
    }
    return true;
}

void PatientParsePage::showFullText()
{
    QTextEdit* fullTextEdit = new QTextEdit;
    fullTextEdit->setReadOnly(true);
    fullTextEdit->setPlainText(results.text);
    fullTextEdit->show();
}
