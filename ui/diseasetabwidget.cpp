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
#include <QHBoxLayout>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QRadioButton>
#include <QToolButton>

// Local includes

#include "entityselectionwidget.h"
#include "patient.h"
#include "pathologypropertyinfo.h"
#include "pathologypropertywidget.h"
#include "pathologywidgetgenerator.h"

class PathologyTab : public QWidget
{
public:
    PathologyTab()
        : enabled(false)
    {
        layout = new QFormLayout;
        setLayout(layout);
    }

    virtual void switchEntity(Pathology::Entity entity)
    {
        generator.switchEntity(entity, context(), layout);
        enabled = entity != Pathology::UnknownEntity;
    }

    virtual void save(const Patient::Ptr& p, Disease& disease, Pathology::Entity entity);
    virtual bool load(const Patient::Ptr& p, const Disease& disease);

    virtual PathologyContextInfo::Context context() const = 0;
    virtual QString tabLabel() const = 0;

    QFormLayout*             layout;
    PathologyWidgetGenerator generator;
    bool                     enabled;

protected:

    PropertyList harvestProperties() const;
    void ensurePathology(Disease& disease, PathologyContextInfo::Context context);
    bool loadProperties(const Disease& disease, PathologyContextInfo::Context context);
    bool saveProperties(Disease& disease, PathologyContextInfo::Context context);
};

class TumorprofilTab : public PathologyTab
{
public:
    virtual PathologyContextInfo::Context context() const { return PathologyContextInfo::Tumorprofil; }
    virtual QString tabLabel() const { return QObject::tr("Tumorprofil"); }
};

class BGJ398Tab : public PathologyTab
{
    virtual PathologyContextInfo::Context context() const { return PathologyContextInfo::ScreeningBGJ398; }
    virtual QString tabLabel() const { return QObject::tr("BGJ389"); }
};

class BEZ235Tab : public PathologyTab
{
    virtual PathologyContextInfo::Context context() const { return PathologyContextInfo::ScreeningBEZ235; }
    virtual QString tabLabel() const { return QObject::tr("BEZ235"); }
};

namespace PathologyTabFactory
{
static PathologyTab* create(PathologyContextInfo::Context context)
{
    switch (context)
    {
    case PathologyContextInfo::Tumorprofil:
        return new TumorprofilTab;
    case PathologyContextInfo::ScreeningBGJ398:
        return new BGJ398Tab;
    case PathologyContextInfo::ScreeningBEZ235:
        return new BEZ235Tab;
    default:
        break;
    }
    return 0;
}
}

class DiseaseTabWidget::DiseaseTabWidgetPriv
{
public:
    DiseaseTabWidgetPriv()
        : diseaseTab(0),
          entityWidget(0),
          initialDiagnosisEdit(0),
          sampleFromPrimaryButton(0),
          sampleFromMetastasisButton(0),
          sampleFromLNButton(0),
          sampleOriginGroup(0),
          otherPathologyGroup(0),
          otherButtonsLayout(0),
          hasValidPathology(false)
    {
    }

    QWidget               *diseaseTab;
    EntitySelectionWidget *entityWidget;
    QDateEdit             *initialDiagnosisEdit;
    QRadioButton          *sampleFromPrimaryButton;
    QRadioButton          *sampleFromMetastasisButton;
    QRadioButton          *sampleFromLNButton;
    QButtonGroup          *sampleOriginGroup;
    QButtonGroup          *otherPathologyGroup;
    QHBoxLayout           *otherButtonsLayout;

    QList<PathologyTab*>   tabs;

    bool                   hasValidPathology;

    Patient::Ptr           currentPatient;

    PathologyTab* findTab(PathologyContextInfo::Context context) const
    {
        foreach (PathologyTab* tab, tabs)
        {
            if (tab->context() == context)
            {
                return tab;
            }
        }
        return 0;
    }

    void addOtherButton(PathologyContextInfo::Context context, const QString& label)
    {
        QToolButton* button = new QToolButton();
        button->setText(label);
        otherButtonsLayout->addWidget(button);
        otherPathologyGroup->addButton(button, context);
    }
};

DiseaseTabWidget::DiseaseTabWidget(QWidget *parent) :
    QTabWidget(parent),
    d(new DiseaseTabWidgetPriv)
{
    // --- Disease Tab ---
    d->diseaseTab = new QWidget;
    QFormLayout * diseaseLayout = new QFormLayout;

    d->entityWidget = new EntitySelectionWidget;
    diseaseLayout->addRow(d->entityWidget);

    d->initialDiagnosisEdit = new QDateEdit;
    diseaseLayout->addRow(tr("Erstdiagnose:"), d->initialDiagnosisEdit);

    // Sample origin buttons
    d->sampleFromPrimaryButton = new QRadioButton(tr("aus dem Primärtumor"));
    d->sampleFromLNButton = new QRadioButton(tr("aus einem lokalen Lymphknoten"));
    d->sampleFromMetastasisButton = new QRadioButton(tr("aus einer Metastase"));
    diseaseLayout->addRow(tr("Histologie"), d->sampleFromPrimaryButton);
    diseaseLayout->addRow(QString(), d->sampleFromLNButton);
    diseaseLayout->addRow(QString(), d->sampleFromMetastasisButton);
    d->sampleOriginGroup = new QButtonGroup(this);
    d->sampleOriginGroup->addButton(d->sampleFromPrimaryButton, Pathology::Primary);
    d->sampleOriginGroup->addButton(d->sampleFromLNButton, Pathology::LocalLymphNode);
    d->sampleOriginGroup->addButton(d->sampleFromMetastasisButton, Pathology::Metastasis);

    // Other pathologies buttons
    d->otherButtonsLayout = new QHBoxLayout;
    d->otherPathologyGroup = new QButtonGroup(this);
    d->addOtherButton(PathologyContextInfo::ScreeningBGJ398, tr("BGJ389-Screening"));
    d->addOtherButton(PathologyContextInfo::ScreeningBEZ235, tr("BEZ235-Screening"));
    diseaseLayout->addRow(d->otherButtonsLayout);

    d->diseaseTab->setLayout(diseaseLayout);
    addTab(d->diseaseTab, tr("Erkrankung"));

    // --- Tumorprofil tab ---
    addPathologyTab(PathologyContextInfo::Tumorprofil);

    connect(d->entityWidget, SIGNAL(selectionChanged(Pathology::Entity)),
            this, SLOT(slotEntitySelectionChanged(Pathology::Entity)));
    connect(d->entityWidget, SIGNAL(entityChanged(Pathology::Entity)),
            this, SLOT(slotEntityChanged(Pathology::Entity)));
    connect(d->initialDiagnosisEdit, SIGNAL(dateChanged(QDate)),
            this, SLOT(slotInitialDiagnosisDateChanged(QDate)));
    connect(d->otherPathologyGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(addPathologyTab(int)));

    updatePathologyTabs();
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

    // Clear other tabs + buttons
    PathologyTab* tumorprofilTab = d->findTab(PathologyContextInfo::Tumorprofil);
    foreach (PathologyTab* tab, d->tabs)
    {
        if (tab != tumorprofilTab)
        {
            delete tab;
        }
    }
    d->tabs.clear();
    d->tabs << tumorprofilTab;
    foreach (QAbstractButton* button, d->otherPathologyGroup->buttons())
    {
        button->setEnabled(true);
    }

    d->currentPatient = p;

    if (!d->currentPatient || d->currentPatient->diseases.isEmpty())
    {
        return;
    }

    const Disease& disease = d->currentPatient->diseases.first();
    d->initialDiagnosisEdit->setDate(disease.initialDiagnosis);

    if (!disease.hasPathology())
    {
        return;
    }

    if (d->sampleOriginGroup->checkedButton())
    {
        d->sampleOriginGroup->checkedButton()->setChecked(false);
    }
    // button is checked below from the Tumorprofil path

    d->entityWidget->setEntity(disease.entity());
    // updatePathologyTab was now called by signal, widgets are built
    loadPathologyData();

    // TODO: Set smoking history
}

void DiseaseTabWidget::loadPathologyData()
{
    Disease& disease = d->currentPatient->firstDisease();

    foreach (const Pathology& path, disease.pathologies)
    {
        PathologyContextInfo::Context context = PathologyContextInfo::info(path.context).context;
        addPathologyTab(context);
        PathologyTab* tab = d->findTab(context);
        if (!tab)
        {
            qDebug() << "Unhandled PathologyContext" << path.context;
            continue;
        }
        if (tab->load(d->currentPatient, disease))
        {
            d->hasValidPathology = true;
        }
    }

    // NOTE: Setting SampleOrigin per-pathology is not supported by the UI
    Pathology::SampleOrigin sampleOrigin = Pathology::UnknownOrigin;
    if (disease.hasProfilePathology())
    {
        sampleOrigin = disease.firstProfilePathology().sampleOrigin;
    }
    else if (disease.hasPathology())
    {
        sampleOrigin = disease.firstPathology().sampleOrigin;
    }

    if (d->sampleOriginGroup->button(sampleOrigin))
    {
        d->sampleOriginGroup->button(sampleOrigin)->setChecked(true);
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
        if (d->entityWidget->currentEntity() == Pathology::UnknownEntity)
        {
            return;
        }
        p->diseases << Disease();
    }

    Disease& disease = p->diseases.first();
    disease.initialDiagnosis = d->initialDiagnosisEdit->date();

    foreach (PathologyTab* tab, d->tabs)
    {
        tab->save(p, disease, d->entityWidget->currentEntity());
    }

    // NOTE: Setting SampleOrigin per-pathology is not supported by the UI
    for (int i=0; i<disease.pathologies.size(); ++i)
    {
        disease.pathologies[i].entity = d->entityWidget->currentEntity();
        disease.pathologies[i].sampleOrigin = d->sampleOriginGroup->checkedButton()
            ? (Pathology::SampleOrigin)d->sampleOriginGroup->checkedId() : Pathology::UnknownOrigin;
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
    updatePathologyTabs();
    if (d->hasValidPathology && entity != Pathology::UnknownEntity)
    {
        changeEntity(entity);
    }
}

void DiseaseTabWidget::changeEntity(Pathology::Entity)
{
    if (!d->currentPatient || !d->currentPatient->hasDisease())
    {
        return;
    }

    loadPathologyData();
}

void DiseaseTabWidget::updatePathologyTabs()
{
    foreach (PathologyTab* tab, d->tabs)
    {
        tab->switchEntity(d->entityWidget->currentEntity());
        setTabEnabled(indexOf(tab), tab->enabled);
    }
}

void DiseaseTabWidget::slotInitialDiagnosisDateChanged(const QDate& date)
{
    if (date > QDate::currentDate())
    {
        QDate previousYear = date.addYears(-1);
        if (previousYear < QDate::currentDate())
        {
            d->initialDiagnosisEdit->setDate(previousYear);
        }
    }
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

void DiseaseTabWidget::addPathologyTab(int context)
{
    if (d->findTab((PathologyContextInfo::Context)context))
    {
        return;
    }

    PathologyTab* tab = PathologyTabFactory::create((PathologyContextInfo::Context)context);

    if (!tab)
    {
        return;
    }

    QAbstractButton* button = d->otherPathologyGroup->button(context);
    if (button)
    {
        button->setEnabled(false);
    }

    addTab(tab, tab->tabLabel());
    d->tabs << tab;
    updatePathologyTabs();
    setCurrentWidget(tab);
}

// -------

bool PathologyTab::loadProperties(const Disease& disease, PathologyContextInfo::Context context)
{
    if (!disease.hasPathology(context))
    {
        return false;
    }

    const Pathology& path = disease.firstPathology(context);

    bool hasValidPathology = false;
    foreach (const Property& prop, path.properties)
    {
        PathologyPropertyWidget* w = generator.propertyWidget(prop.property);
        if (!w)
        {
            qWarning() << "Unhandled property" << prop.property;
            continue;
        }
        hasValidPathology = true;
        w->setValue(prop);
    }
    return hasValidPathology;
}

PropertyList PathologyTab::harvestProperties() const
{
    PropertyList properties;
    foreach (PathologyPropertyWidget* w, generator.m_widgets)
    {
        Property prop = w->currentProperty();
        if (!prop.isEmpty())
        {
            properties << prop;
        }
    }
    return properties;
}

void PathologyTab::ensurePathology(Disease& disease, PathologyContextInfo::Context context)
{
    if (!disease.hasPathology(context))
    {
        Pathology path;
        path.context = PathologyContextInfo(context).id;
        disease.pathologies << path;
    }
}
bool PathologyTab::saveProperties(Disease& disease, PathologyContextInfo::Context context)
{
    PropertyList profileProperties = harvestProperties();
    if (profileProperties.isEmpty())
    {
        return false;
    }
    ensurePathology(disease, context);
    Pathology& path = disease.firstPathology(context);
    // Attention: does not preserve unhandled properties
    path.properties = profileProperties;
    return true;
}

bool PathologyTab::load(const Patient::Ptr& p, const Disease& disease)
{
    return loadProperties(disease, context());
}

void PathologyTab::save(const Patient::Ptr& p, Disease& disease, Pathology::Entity entity)
{
    saveProperties(disease, context());
}
