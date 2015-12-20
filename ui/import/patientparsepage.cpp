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

#include <QAction>
#include <QDateEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QMenu>
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
        QHBoxLayout* idLayout = new QHBoxLayout;
        idLayout->addWidget(new QLabel(tr("Erstdiagnose:")));
        initialDiagnosisEdit = new QDateEdit;
        idLayout->addWidget(initialDiagnosisEdit);
        idLayout->addStretch(1);

        QDate date;
        date.setDate(results.resultsDate.year(), results.resultsDate.month(), 1);
        if (date.daysTo(results.resultsDate) < 10)
        {
            date = date.addMonths(-1);
        }
        initialDiagnosisEdit->setDate(date);

        layout->addLayout(idLayout);
    }

    model = new PathologyPropertiesTableModel(this);
    model->setEditingEnabled(true);
    filterModel = new PathologyPropertiesTableFilterModel(this);
    view  = new PathologyPropertiesTableView;
    view->setModels(model, filterModel);
    layout->addWidget(view, 2);

    Pathology path;
    path.properties = results.properties;
    path.date       = results.resultsDate;
    model->setPathology(path);

    QHBoxLayout* lineBelowTableView = new QHBoxLayout;

    QLabel* unknownTextLabel = new QLabel(tr("Nicht erkannte Passagen aus den Befunden:"));
    lineBelowTableView->addWidget(unknownTextLabel);

    lineBelowTableView->addStretch(1);
    QPushButton* addNewFindingButton = new QPushButton(QIcon::fromTheme("add"), tr("Eintrag hinzufügen"));
    addNewFindingButton->setMenu(buildPropertyMenu());
    connect(addNewFindingButton->menu(), &QMenu::triggered, this, &PatientParsePage::addPropertyTriggered);
    lineBelowTableView->addWidget(addNewFindingButton);

    layout->addLayout(lineBelowTableView);

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
        newPath.entity = entitySelectionWidget->entity();
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

QMenu* PatientParsePage::buildPropertyMenu()
{
    QMenu* menu = new QMenu;

    QMenu* ihcMenu = menu->addMenu(tr("IHC"));
    fillPropertySubmenu(ihcMenu, PathologyPropertyInfo::allIHC());

    QMenu* mutationSubMenu = menu->addMenu(tr("Mutationsanalyse"));
    fillPropertySubmenu(mutationSubMenu, PathologyPropertyInfo::allMutations());

    QMenu* fishSubMenu = menu->addMenu(tr("FISH"));
    fillPropertySubmenu(fishSubMenu, PathologyPropertyInfo::allFish());

    QAction* allAdenoAction = ihcMenu->addAction(tr("IHC Adenokarzinom Lunge"));
    allAdenoAction->setData(QVariantList()
                            << PathologyPropertyInfo::IHC_ALK
                            << PathologyPropertyInfo::IHC_ROS1
                            << PathologyPropertyInfo::IHC_PTEN
                            << PathologyPropertyInfo::IHC_HER2
                            << PathologyPropertyInfo::IHC_HER2_DAKO
                            << PathologyPropertyInfo::IHC_pAKT
                            << PathologyPropertyInfo::IHC_pERK
                            << PathologyPropertyInfo::IHC_PDL1
                            << PathologyPropertyInfo::IHC_PDL1_immunecell);

    QAction* allPECAction = ihcMenu->addAction(tr("IHC PEC Lunge"));
    allPECAction->setData(QVariantList()
                            << PathologyPropertyInfo::IHC_PTEN
                            << PathologyPropertyInfo::IHC_pAKT
                            << PathologyPropertyInfo::IHC_pERK);

    QAction* allCRCAction = ihcMenu->addAction(tr("IHC CRC"));
    allCRCAction->setData(QVariantList()
                            << PathologyPropertyInfo::IHC_PTEN
                            << PathologyPropertyInfo::IHC_pAKT
                            << PathologyPropertyInfo::IHC_pERK
                            << PathologyPropertyInfo::IHC_pP70S6K
                            << PathologyPropertyInfo::IHC_MLH1
                            << PathologyPropertyInfo::IHC_MSH2);

    QAction* allMutAction = mutationSubMenu->addAction(tr("NGS Panel"));
    allMutAction->setData(QVariantList()
                          << PathologyPropertyInfo::Mut_BRAF_11
                          << PathologyPropertyInfo::Mut_BRAF_15
                          << PathologyPropertyInfo::Mut_DDR2
                          << PathologyPropertyInfo::Mut_EGFR_18_20
                          << PathologyPropertyInfo::Mut_EGFR_19_21
                          << PathologyPropertyInfo::Mut_ERBB2
                          << PathologyPropertyInfo::Mut_FGFR1
                          << PathologyPropertyInfo::Mut_FGFR3
                          << PathologyPropertyInfo::Mut_HRAS_2_4
                          << PathologyPropertyInfo::Mut_NRAS_2_4
                          << PathologyPropertyInfo::Mut_KIT
                          << PathologyPropertyInfo::Mut_KRAS_2
                          << PathologyPropertyInfo::Mut_KRAS_3
                          << PathologyPropertyInfo::Mut_KRAS_4
                          << PathologyPropertyInfo::Mut_MET
                          << PathologyPropertyInfo::Mut_PDGFRa
                          << PathologyPropertyInfo::Mut_PIK3CA_10_21
                          << PathologyPropertyInfo::Mut_RET
                          << PathologyPropertyInfo::Mut_TP53);

    return menu;
}

void PatientParsePage::fillPropertySubmenu(QMenu* menu, const QList<PathologyPropertyInfo>& infos)
{
    foreach (const PathologyPropertyInfo& info, infos)
    {
        QAction* action = menu->addAction(info.label);
        action->setData((int)info.property);
    }
}

void PatientParsePage::addPropertyTriggered(QAction *action)
{
    QVariant propertyData = action->data();
    QList<PathologyPropertyInfo> infos;
    if (propertyData.type() == QVariant::Int)
    {
        infos << PathologyPropertyInfo::info((PathologyPropertyInfo::Property)propertyData.toInt());
    }
    else if (propertyData.type() == QVariant::List)
    {
        foreach (const QVariant& element, propertyData.toList())
        {
            infos << PathologyPropertyInfo::info((PathologyPropertyInfo::Property)element.toInt());
        }
    }

    PropertyList newProperties;
    foreach (const PathologyPropertyInfo& info, infos)
    {
        Property prop;
        prop.property = info.id;
        newProperties << prop;
    }
    model->addProperties(newProperties);
}
