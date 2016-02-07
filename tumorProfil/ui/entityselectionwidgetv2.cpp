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
        : mode(EntitySelectionWidgetV2::DisplayMode),
          entity(Pathology::UnknownEntity),
          comboBox(0)
    {
        entries.insert(Pathology::PulmonaryAdeno, tr("Adenokarzinom der Lunge"));
        entries.insert(Pathology::PulmonarySquamous, tr("Plattenepithelkarzinom der Lunge"));
        entries.insert(Pathology::PulmonaryOtherCarcinoma, tr("Andere Karzinome der Lunge"));
        entries.insert(Pathology::ColorectalAdeno, tr("Kolorektales Karzinom"));
        entries.insert(Pathology::Esophageal, tr("Ösophaguskarzinom"));
        entries.insert(Pathology::EsophagogastrealJunction, tr("Ösophagogastrales Karzinom"));
        entries.insert(Pathology::Gastric, tr("Magenkarzinom"));
        entries.insert(Pathology::Cholangiocarcinoma, tr("Gallengangskarzinom"));
        entries.insert(Pathology::Breast, tr("Mammakarzinom"));
        entries.insert(Pathology::RenalCell, tr("Nierenzellkarzinom"));
        entries.insert(Pathology::TransitionalCell, tr("Urothelkarzinom"));
        entries.insert(Pathology::Thyroid, tr("Schilddrüsenkarzinom"));
        entries.insert(Pathology::Melanoma, tr("Malignes Melanom"));
    }

    void addButton(EntitySelectionWidgetV2* q, Pathology::Entity entity, QGridLayout* layout, int row, int column)
    {
        QToolButton* button = new QToolButton;
        QAction* action = new QAction(entries.value(entity), q);
        action->setCheckable(true);
        button->setDefaultAction(action);
        layout->addWidget(button, row, column);
        QObject::connect(button, SIGNAL(triggered(QAction*)), q, SLOT(buttonPressed(QAction*)));
        action->setData((int)entity);
        buttons << button;
    }

    QMap<Pathology::Entity,QString> entries;

    EntitySelectionWidgetV2::Mode mode;

    Pathology::Entity entity;
    QLabel*           label;
    QList<QToolButton*> buttons;
    QComboBox*        comboBox;
};

EntitySelectionWidgetV2::EntitySelectionWidgetV2(QWidget *parent)
    : QWidget(parent),
      d(new EntitySelectionWidgetV2Priv)
{
    QGridLayout* layout = new QGridLayout;

    QLabel *icon = new QLabel;
    icon->setPixmap(QIcon::fromTheme("arrow_divide").pixmap(16,16));
    QLabel *explanation = new QLabel("Entität:");
    d->label = new QLabel;
    connect(d->label, &QLabel::linkActivated, this, &EntitySelectionWidgetV2::linkPressed);
    QHBoxLayout* labelLayout = new QHBoxLayout;
    labelLayout->addWidget(icon);
    labelLayout->addWidget(explanation);
    labelLayout->addWidget(d->label);
    labelLayout->addStretch();

    layout->addLayout(labelLayout, 0, 0, 1, 2);

    d->addButton(this, Pathology::PulmonaryAdeno, layout, 1, 0);
    d->addButton(this, Pathology::PulmonarySquamous, layout, 1, 1);
    d->addButton(this, Pathology::ColorectalAdeno, layout, 2, 0);

    d->comboBox = new QComboBox;
    for (QMap<Pathology::Entity, QString>::const_iterator it = d->entries.begin(); it != d->entries.end(); ++it)
    {
        d->comboBox->addItem(it.value(), it.key());
    }

    connect(d->comboBox, (void(QComboBox::*)(int))&QComboBox::currentIndexChanged, this, &EntitySelectionWidgetV2::comboBoxIndexChanged);
    layout->addWidget(d->comboBox, 2, 1);
    setLayout(layout);

    applyMode();
}

EntitySelectionWidgetV2::~EntitySelectionWidgetV2()
{
    delete d;
}

Pathology::Entity EntitySelectionWidgetV2::entity() const
{
    return d->entity;
}

EntitySelectionWidgetV2::Mode EntitySelectionWidgetV2::mode() const
{
    return d->mode;
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
    applyLabelState();
    emit entityChanged(d->entity);
}

void EntitySelectionWidgetV2::setMode(Mode mode)
{
    if (d->mode == mode)
    {
        return;
    }

    d->mode = mode;
    applyMode();
}

void EntitySelectionWidgetV2::comboBoxIndexChanged(int index)
{
    setEntity(static_cast<Pathology::Entity>(d->comboBox->itemData(index).toInt()));
}

void EntitySelectionWidgetV2::buttonPressed(QAction* action)
{
    setEntity(static_cast<Pathology::Entity>(action->data().toInt()));
}

void EntitySelectionWidgetV2::linkPressed(const QString &)
{
    if (d->mode == EditMode)
    {
        setMode(DisplayMode);
    }
    else
    {
        setMode(EditMode);
    }
}

void EntitySelectionWidgetV2::applyLabelState()
{
    if (d->entity == Pathology::UnknownEntity)
    {
        d->label->setText(QString("<html><style type=\"text/css\">a:link {color:%1;text-decoration:none}</style><a href='entity'>%2</a></html>")
                          .arg(palette().text().color().name(QColor::HexRgb)).arg(tr("Unbekannt")));
    }
    else
    {
        d->label->setText(QString("<html><style type=\"text/css\">a:link {color:%1;}</style> <a href='entity'>%2</a></html>")
                          .arg(palette().text().color().name(QColor::HexRgb)).arg(d->entries.value(d->entity)));
    }
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

void EntitySelectionWidgetV2::applyMode()
{
    foreach (QToolButton* button, d->buttons)
    {
        button->setVisible(d->mode == EditMode);
    }
    d->comboBox->setVisible(d->mode == EditMode);
}

QGroupBox* EntitySelectionWidgetV2::createGroupBox()
{
    QGroupBox* box = new QGroupBox;
    QVBoxLayout* boxlayout = new QVBoxLayout;
    boxlayout->addWidget(this);
    box->setLayout(boxlayout);
    return box;
}
