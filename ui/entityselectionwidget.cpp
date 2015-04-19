/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 31.01.2012
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

#include "entityselectionwidget.h"

// Qt includes

#include <QButtonGroup>
#include <QDebug>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>

class EntitySelectionWidget::EntitySelectionWidgetPriv
{
public:
    EntitySelectionWidgetPriv()
        : entity(Pathology::UnknownEntity)
    {
    }

    Pathology::Entity entity;

    QVBoxLayout *layout;
    QButtonGroup *buttonGroup;

    void addRadioButton(Pathology::Entity entity, const QString& label, QVBoxLayout* layout)
    {
        QRadioButton* r = new QRadioButton(label);
        layout->addWidget(r);
        buttonGroup->addButton(r, entity);
    }
};

EntitySelectionWidget::EntitySelectionWidget(QWidget *parent) :
    QGroupBox(parent),
    d(new EntitySelectionWidgetPriv)
{
    QGridLayout* mainLayout = new QGridLayout;
    d->buttonGroup = new QButtonGroup(this);
    d->buttonGroup->setExclusive(true);
    connect(d->buttonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(selectionChanged(int)));

    QLabel *icon = new QLabel;
    icon->setPixmap(QIcon::fromTheme("arrow_divide").pixmap(16,16));
    QLabel *explanation = new QLabel("Entität:");
    QHBoxLayout* labelLayout = new QHBoxLayout;
    labelLayout->addWidget(icon);
    labelLayout->addWidget(explanation);
    labelLayout->addStretch();

    QGroupBox* pulmoGroup = new QGroupBox(QObject::tr("Lunge"));
    QGroupBox* giGroup = new QGroupBox(QObject::tr("Gastrointestinal"));
    QGroupBox* otherGroup = new QGroupBox(QObject::tr("Andere"));

    QVBoxLayout* pulmoLayout = new QVBoxLayout;
    QVBoxLayout* giLayout = new QVBoxLayout;
    QVBoxLayout* otherLayout = new QVBoxLayout;

    d->addRadioButton(Pathology::PulmonaryAdeno, tr("Pulmonales Adenokarzinom"), pulmoLayout);
    d->addRadioButton(Pathology::PulmonarySquamous, tr("Pulmonales Plattenepithelkarzinom"), pulmoLayout);
    pulmoLayout->addSpacing(10);
    d->addRadioButton(Pathology::PulmonaryLargeCell, tr("Pulmonales großzelliges Karzinom"), pulmoLayout);
    d->addRadioButton(Pathology::PulmonaryAdenosquamous, tr("Pulmonales adenosquamöses Karzinom"), pulmoLayout);
    d->addRadioButton(Pathology::PulmonaryBronchoalveloar, tr("Bronchoalveoläres Karzinom"), pulmoLayout);
    d->addRadioButton(Pathology::PulmonaryOtherCarcinoma, tr("Andere Karzinome der Lunge"), pulmoLayout);
    pulmoLayout->addStretch(1);

    d->addRadioButton(Pathology::ColorectalAdeno, tr("Kolorektales Karzinom"), giLayout);
    giLayout->addSpacing(10);
    d->addRadioButton(Pathology::Esophageal, tr("Ösophaguskarzinom"), giLayout);
    d->addRadioButton(Pathology::EsophagogastrealJunction, tr("Ösophagogastrales Karzinom"), giLayout);
    d->addRadioButton(Pathology::Gastric, tr("Magenkarzinom"), giLayout);
    d->addRadioButton(Pathology::Cholangiocarcinoma, tr("Gallengangskarzinom"), giLayout);
    giLayout->addStretch(1);

    d->addRadioButton(Pathology::Breast, tr("Mammakarzinom"), otherLayout);
    d->addRadioButton(Pathology::RenalCell, tr("Nierenzellkarzinom"), otherLayout);
    d->addRadioButton(Pathology::TransitionalCell, tr("Urothelkarzinom"), otherLayout);
    d->addRadioButton(Pathology::Thyroid, tr("Schilddrüsenkarzinom"), otherLayout);
    d->addRadioButton(Pathology::Melanoma, tr("Malignes Melanom"), otherLayout);
    otherLayout->addStretch(1);

    pulmoGroup->setLayout(pulmoLayout);
    giGroup->setLayout(giLayout);
    otherGroup->setLayout(otherLayout);

    mainLayout->addLayout(labelLayout, 0, 0, 1, 2);
    mainLayout->addWidget(pulmoGroup, 1, 0);
    mainLayout->addWidget(giGroup, 1, 2);
    mainLayout->addWidget(otherGroup, 2, 0, 1, 2);
    mainLayout->setColumnStretch(3, 1);

    setLayout(mainLayout);
}

EntitySelectionWidget::~EntitySelectionWidget()
{
    delete d;
}

Pathology::Entity EntitySelectionWidget::currentEntity() const
{
    return d->entity;
}

void EntitySelectionWidget::setEntity(Pathology::Entity entity)
{
    if (entity == d->entity)
    {
        return;
    }

    d->entity = entity;
    QAbstractButton* r = d->buttonGroup->button(entity);
    if (r)
    {
        r->setChecked(true);
    }
    else if (d->buttonGroup->checkedButton())
    {
        // uncheck all
        d->buttonGroup->setExclusive(false);
        d->buttonGroup->checkedButton()->setChecked(false);
        d->buttonGroup->setExclusive(true);
    }
    emit entityChanged(d->entity);
}

void EntitySelectionWidget::selectionChanged(int id)
{
    if (id == d->entity)
    {
        return;
    }
    emit selectionChanged((Pathology::Entity)id);
}

void EntitySelectionWidget::applySelectionChange()
{
    setEntity((Pathology::Entity)d->buttonGroup->checkedId());
}

void EntitySelectionWidget::discardSelectionChange()
{
    QAbstractButton* r = d->buttonGroup->button(d->entity);
    if (r)
    {
        r->setChecked(true);
    }
}


