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

#include "diseasetabwidget.h"

// Qt includes

#include <QButtonGroup>
#include <QDateEdit>
#include <QDebug>
#include <QFormLayout>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QRadioButton>

// Local includes

#include "entityselectionwidget.h"
#include "patient.h"
#include "pathologypropertywidget.h"
#include "pathologywidgetgenerator.h"

class DiseaseTabWidget::DiseaseTabWidgetPriv
{
public:
    DiseaseTabWidgetPriv()
        : diseaseTab(0),
          entityWidget(0),
          initialDiagnosisEdit(0),
          sampleFromPrimaryButton(0),
          sampleFromMetastasisButton(0),
          sampleOriginGroup(0),
          pathologyTab(0),
          pathologyLayout(0),
          hasValidPathology(false)
    {
    }

    QWidget               *diseaseTab;
    EntitySelectionWidget *entityWidget;
    QDateEdit             *initialDiagnosisEdit;
    QRadioButton          *sampleFromPrimaryButton;
    QRadioButton          *sampleFromMetastasisButton;
    QButtonGroup          *sampleOriginGroup;

    QWidget               *pathologyTab;
    QFormLayout           *pathologyLayout;
    PathologyWidgetGenerator generator;

    bool                   hasValidPathology;

    Patient::Ptr           currentPatient;
};

DiseaseTabWidget::DiseaseTabWidget(QWidget *parent) :
    QTabWidget(parent),
    d(new DiseaseTabWidgetPriv)
{
    d->diseaseTab = new QWidget;
    QFormLayout * diseaseLayout = new QFormLayout;
    d->entityWidget = new EntitySelectionWidget;
    d->initialDiagnosisEdit = new QDateEdit;
    diseaseLayout->addRow(d->entityWidget);
    diseaseLayout->addRow(tr("Erstdiagnose:"), d->initialDiagnosisEdit);
    d->sampleFromPrimaryButton = new QRadioButton(tr("aus dem Primärtumor"));
    d->sampleFromMetastasisButton = new QRadioButton(tr("aus einer Metastase"));
    diseaseLayout->addRow(tr("Histologie"), d->sampleFromPrimaryButton);
    diseaseLayout->addRow(QString(), d->sampleFromMetastasisButton);
    d->diseaseTab->setLayout(diseaseLayout);
    d->sampleOriginGroup = new QButtonGroup(this);
    d->sampleOriginGroup->addButton(d->sampleFromPrimaryButton, Pathology::Primary);
    d->sampleOriginGroup->addButton(d->sampleFromMetastasisButton, Pathology::Metastasis);
    d->pathologyTab = new QWidget;
    d->pathologyLayout = new QFormLayout;
    d->pathologyTab->setLayout(d->pathologyLayout);

    addTab(d->diseaseTab, tr("Erkrankung"));
    addTab(d->pathologyTab, tr("Pathologie"));

    connect(d->entityWidget, SIGNAL(selectionChanged(Pathology::Entity)),
            this, SLOT(slotEntitySelectionChanged(Pathology::Entity)));
    connect(d->entityWidget, SIGNAL(entityChanged(Pathology::Entity)),
            this, SLOT(slotEntityChanged(Pathology::Entity)));

    updatePathologyTab();
}

DiseaseTabWidget::~DiseaseTabWidget()
{
    delete d;
}

void DiseaseTabWidget::setPatient(const Patient::Ptr& p)
{
    d->initialDiagnosisEdit->setDate(QDate::currentDate());
    d->entityWidget->setEntity(Pathology::UnknownEntity);
    if (d->sampleOriginGroup->checkedButton())
    {
        d->sampleOriginGroup->setExclusive(false);
        d->sampleOriginGroup->checkedButton()->setChecked(false);
        d->sampleOriginGroup->setExclusive(true);
    }
    d->hasValidPathology = false;

    d->currentPatient = p;

    if (!d->currentPatient || d->currentPatient->diseases.isEmpty())
    {
        return;
    }

    const Disease& disease = d->currentPatient->diseases.first();
    d->initialDiagnosisEdit->setDate(disease.initialDiagnosis);

    if (disease.pathologies.isEmpty())
    {
        return;
    }
    const Pathology& path = disease.pathologies.first();

    if (d->sampleOriginGroup->checkedButton())
    {
        d->sampleOriginGroup->checkedButton()->setChecked(false);
    }
    if (d->sampleOriginGroup->button(path.sampleOrigin))
    {
        d->sampleOriginGroup->button(path.sampleOrigin)->setChecked(true);
    }

    d->entityWidget->setEntity(path.entity);
    // updatePathologyTab was now called by signal, widgets are built
    updatePathologyProperties(path);

    // TODO: Set smoking history
}

void DiseaseTabWidget::updatePathologyProperties(const Pathology& path)
{
    foreach (const Property& prop, path.properties)
    {
        PathologyPropertyWidget* w = d->generator.propertyWidget(prop.property);
        if (!w)
        {
            qWarning() << "Unhandled property" << prop.property;
            continue;
        }
        d->hasValidPathology = true;
        w->setValue(prop);
    }
}

void DiseaseTabWidget::save(const Patient::Ptr& p)
{
    if (!p)
    {
        return;
    }

    if (p->diseases.isEmpty())
    {
        p->diseases << Disease();
    }

    Disease& disease = p->diseases.first();
    disease.initialDiagnosis = d->initialDiagnosisEdit->date();

    if (disease.pathologies.isEmpty())
    {
        disease.pathologies << Pathology();
    }

    Pathology& path = disease.pathologies.first();
    path.entity = d->entityWidget->currentEntity();
    path.sampleOrigin = d->sampleOriginGroup->checkedButton()
            ? (Pathology::SampleOrigin)d->sampleOriginGroup->checkedId() : Pathology::UnknownOrigin;

    path.properties.clear(); // Attention: does not preserve unhandled properties
    foreach (PathologyPropertyWidget* w, d->generator.m_widgets)
    {
        Property p = w->currentProperty();
        if (!p.isEmpty())
        {
            path.properties << p;
        }
    }
}

void DiseaseTabWidget::slotEntitySelectionChanged(Pathology::Entity entity)
{
    if (d->hasValidPathology)
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Warning);
        box.setText(tr("Die histologische Entität wurde geändert, "
                       "obwohl bereits Befunde eingegeben wurden."));
        box.setInformativeText(tr("Möchten sie die Entität ändern? "
                                  "Einige Befunde werden möglicherweise fortfallen."));
        box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        box.button(QMessageBox::Yes)->setText(tr("Ja"));
        box.button(QMessageBox::No)->setText(tr("Nein"));
        box.setDefaultButton(QMessageBox::Yes);
        if (box.exec() == QMessageBox::No)
        {
            d->entityWidget->discardSelectionChange();
            return;
        }
    }
    d->entityWidget->applySelectionChange();
    // will emit entityChanged, will call updatePathologyTab
}

void DiseaseTabWidget::slotEntityChanged(Pathology::Entity entity)
{
    updatePathologyTab();
    if (d->hasValidPathology && entity != Pathology::UnknownEntity)
    {
        changeEntity(entity);
    }
}

void DiseaseTabWidget::changeEntity(Pathology::Entity)
{
    if (!d->currentPatient || d->currentPatient->diseases.isEmpty())
    {
        return;
    }
    const Disease& disease = d->currentPatient->diseases.first();
    if (disease.pathologies.isEmpty())
    {
        return;
    }
    const Pathology& path = disease.pathologies.first();

    updatePathologyProperties(path);
}

void DiseaseTabWidget::updatePathologyTab()
{
    d->generator.switchEntity(d->entityWidget->currentEntity(), d->pathologyLayout);
    setTabEnabled(indexOf(d->pathologyTab),
                  d->entityWidget->currentEntity() != Pathology::UnknownEntity);
}

void DiseaseTabWidget::keyPressEvent(QKeyEvent* e)
{
    switch (e->key())
    {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        emit editingFinished();
        return;
    }
    QWidget::keyPressEvent(e);
}
