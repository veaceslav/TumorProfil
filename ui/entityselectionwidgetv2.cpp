/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 17.06.2015
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

#include "entityselectionwidgetv2.h"

#include <QAction>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>

class EntitySelectionWidgetV2::EntitySelectionWidgetV2Priv
{
public:

    EntitySelectionWidgetV2Priv()
        : entity(Pathology::UnknownEntity),
          comboBox(0)
    {
    }

    void addButton(EntitySelectionWidgetV2* q, const QString& label, QGridLayout* layout, int row, int column, Pathology::Entity entity)
    {
        QToolButton* button = new QToolButton;
        QAction* action = new QAction(label, q);
        action->setCheckable(true);
        button->setDefaultAction(action);
        layout->addWidget(button, row, column);
        QObject::connect(button, SIGNAL(triggered(QAction*)), q, SLOT(buttonPressed(QAction*)));
        action->setData((int)entity);
    }

    Pathology::Entity entity;
    QComboBox*        comboBox;
};

EntitySelectionWidgetV2::EntitySelectionWidgetV2(QWidget *parent)
    : QWidget(parent),
      d(new EntitySelectionWidgetV2Priv)
{
    QGridLayout* layout = new QGridLayout;
    d->addButton(this, tr("Adenokarzinom der Lunge"), layout, 0, 0, Pathology::PulmonaryAdeno);
    d->addButton(this, tr("Plattenepithelkarzinom der Lunge"), layout, 0, 1, Pathology::PulmonarySquamous);
    d->addButton(this, tr("Kolorektales Karzinom"), layout, 1, 0, Pathology::ColorectalAdeno);

    d->comboBox = new QComboBox;
    d->comboBox->addItem(tr("Pulmonales Adenokarzinom"), Pathology::PulmonaryAdeno);
    d->comboBox->addItem(tr("Pulmonales Plattenepithelkarzinom"), Pathology::PulmonarySquamous);
    d->comboBox->addItem(tr("Andere Karzinome der Lunge"), Pathology::PulmonaryOtherCarcinoma);
    d->comboBox->addItem(tr("Kolorektales Karzinom"), Pathology::ColorectalAdeno);
    d->comboBox->addItem(tr("Ösophaguskarzinom"), Pathology::Esophageal);
    d->comboBox->addItem(tr("Ösophagogastrales Karzinom"), Pathology::EsophagogastrealJunction);
    d->comboBox->addItem(tr("Magenkarzinom"), Pathology::Gastric);
    d->comboBox->addItem(tr("Gallengangskarzinom"), Pathology::Cholangiocarcinoma);
    d->comboBox->addItem(tr("Mammakarzinom"), Pathology::Breast);
    d->comboBox->addItem(tr("Nierenzellkarzinom"), Pathology::RenalCell);
    d->comboBox->addItem(tr("Urothelkarzinom"), Pathology::TransitionalCell);
    d->comboBox->addItem(tr("Schilddrüsenkarzinom"), Pathology::Thyroid);
    d->comboBox->addItem(tr("Malignes Melanom"), Pathology::Melanoma);

    connect(d->comboBox, (void(QComboBox::*)(int))&QComboBox::currentIndexChanged, this, &EntitySelectionWidgetV2::comboBoxIndexChanged);
    layout->addWidget(d->comboBox, 1, 1);
    setLayout(layout);
}

EntitySelectionWidgetV2::~EntitySelectionWidgetV2()
{
    delete d;
}

Pathology::Entity EntitySelectionWidgetV2::entity() const
{
    return d->entity;
}

void EntitySelectionWidgetV2::setEntity(Pathology::Entity entity)
{
    if (d->entity == entity)
    {
        return;
    }
    d->entity = entity;
    applyButtonState();
    applyComboBoxState();
    emit entityChanged(d->entity);
}

void EntitySelectionWidgetV2::comboBoxIndexChanged(int index)
{
    setEntity(static_cast<Pathology::Entity>(d->comboBox->itemData(index).toInt()));
}

void EntitySelectionWidgetV2::buttonPressed(QAction* action)
{
    setEntity(static_cast<Pathology::Entity>(action->data().toInt()));
}

void EntitySelectionWidgetV2::applyButtonState()
{
    foreach (QToolButton* button, findChildren<QToolButton*>())
    {
        QAction* action = button->defaultAction();
        if (!action)
            continue;
        button->setChecked(action->data().toInt() == d->entity);
    }
}

void EntitySelectionWidgetV2::applyComboBoxState()
{
    int index = d->comboBox->findData((int)d->entity);
    d->comboBox->setCurrentIndex(index);
}

QGroupBox* EntitySelectionWidgetV2::createGroupBox()
{
    QGroupBox* box = new QGroupBox;
    QVBoxLayout* boxlayout = new QVBoxLayout;

    QLabel *icon = new QLabel;
    icon->setPixmap(QIcon::fromTheme("arrow_divide").pixmap(16,16));
    QLabel *explanation = new QLabel("Entität:");
    QHBoxLayout* labelLayout = new QHBoxLayout;
    labelLayout->addWidget(icon);
    labelLayout->addWidget(explanation);
    labelLayout->addStretch();

    boxlayout->addLayout(labelLayout);
    boxlayout->addWidget(this);

    box->setLayout(boxlayout);
    return box;
}
