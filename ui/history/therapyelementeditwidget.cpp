/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 23.05.2013
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

#include "therapyelementeditwidget.h"

// Qt includes

#include <QButtonGroup>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

TherapyElementEditWidget::TherapyElementEditWidget(QWidget *parent)
    : QWidget(parent),
      m_element(0)
{
    therapyLayout = new QVBoxLayout;

    QHBoxLayout* firstLineLayout = new QHBoxLayout;
    therapyLabel  = new QLabel;
    clearButton   = new QPushButton(QIcon::fromTheme("delete"), QString());
    firstLineLayout->addWidget(therapyLabel);
    firstLineLayout->addStretch();
    firstLineLayout->addWidget(clearButton);
    therapyLayout->addLayout(firstLineLayout);
    connect(clearButton, SIGNAL(clicked()), this, SIGNAL(remove()));

    setLayout(therapyLayout);
}

TherapyElementEditWidget::~TherapyElementEditWidget()
{
}

TherapyElementEditWidget* TherapyElementEditWidget::create(TherapyElement* te)
{
    TherapyElementEditWidget* tee = 0;
    if (te->is<Chemotherapy>())
    {
        tee = new CTxEditWidget;
    }
    else if (te->is<Radiotherapy>())
    {
        tee = new RTxEditWidget;
    }
    else if (te->is<Toxicity>())
    {
        tee = new ToxicityEditWidget;
    }

    if (tee)
    {
        tee->setElement(te);
    }
    return tee;
}

TherapyElement* TherapyElementEditWidget::element() const
{
    return m_element;
}

TherapyElement* TherapyElementEditWidget::applyToElement() const
{
    return m_element;
}

void TherapyElementEditWidget::setElement(TherapyElement* element)
{
    m_element = element;
}

namespace {
enum
{
    Relative = 1,
    Absolute = 2
};
}

CTxEditWidget::CTxEditWidget(QWidget* parent)
{
    therapyLabel->setText(tr("Substanz"));

    QHBoxLayout* substanceLayout = new QHBoxLayout;
    substanceBox = new QComboBox;
    substanceBox->addItems(Chemotherapy::substances());
    substanceBox->setEditable(true);
    substanceBox->setInsertPolicy(QComboBox::NoInsert);
    doseNumber   = new QLineEdit;
    QIntValidator* intValidator = new QIntValidator;
    doseNumber->setValidator(intValidator);
    relAbsGroup = new QButtonGroup(this);
    QRadioButton* mgm2 = new QRadioButton(tr("mg/m²"));
    QRadioButton* mgabs = new QRadioButton(tr("mg abs."));
    relAbsGroup->addButton(mgm2, Relative);
    relAbsGroup->addButton(mgabs, Absolute);
    mgm2->setChecked(true);
    substanceLayout->addWidget(substanceBox);
    substanceLayout->addWidget(doseNumber);
    substanceLayout->addWidget(mgm2);
    substanceLayout->addWidget(mgabs);

    QHBoxLayout* daysLayout = new QHBoxLayout;
    QLabel* daysLabel = new QLabel(tr("Tage"));
    scheduleLineEdit = new QLineEdit;
    daysLayout->addWidget(daysLabel);
    daysLayout->addWidget(scheduleLineEdit);

    therapyLayout->addLayout(substanceLayout);
    therapyLayout->addLayout(daysLayout);

    connect(substanceBox, SIGNAL(currentIndexChanged(int)), this, SIGNAL(changed()));
    connect(substanceBox, SIGNAL(editTextChanged(QString)), this, SIGNAL(changed()));
    connect(doseNumber, SIGNAL(editingFinished()), this, SIGNAL(changed()));
}

TherapyElement* CTxEditWidget::applyToElement() const
{
    Chemotherapy* ctx = m_element->as<Chemotherapy>();
    ctx->substance = substanceBox->currentText();
    if (relAbsGroup->checkedId() == Absolute)
    {
        ctx->absdose = doseNumber->text().toInt();
        ctx->dose    = 0;
    }
    else
    {
        ctx->absdose = 0;
        ctx->dose    = doseNumber->text().toInt();
    }
    ctx->schedule = scheduleLineEdit->text();
    return TherapyElementEditWidget::applyToElement();
}

void CTxEditWidget::setElement(TherapyElement* element)
{
    TherapyElementEditWidget::setElement(element);
    Chemotherapy* ctx = m_element->as<Chemotherapy>();
    int index = substanceBox->findText(ctx->substance);
    if (index == -1)
    {
        substanceBox->lineEdit()->setText(ctx->substance);
    }
    else
    {
        substanceBox->setCurrentIndex(index);
    }
    if (ctx->dose)
    {
        relAbsGroup->button(Relative)->setChecked(true);
        doseNumber->setText(QString::number(ctx->dose));
    }
    else if (ctx->absdose)
    {
        relAbsGroup->button(Absolute)->setChecked(true);
        doseNumber->setText(QString::number(ctx->absdose));
    }
    else
    {
        relAbsGroup->button(Relative)->setChecked(true);
        doseNumber->clear();
    }
    scheduleLineEdit->setText(ctx->schedule);
}

RTxEditWidget::RTxEditWidget(QWidget* parent)
{
    therapyLabel->setText(tr("Bestrahlung"));

    QFormLayout* layout = new QFormLayout;
    QHBoxLayout* doseLayout = new QHBoxLayout;
    doseNumberEdit = new QLineEdit;
    QIntValidator* intValidator = new QIntValidator;
    doseNumberEdit->setValidator(intValidator);
    QLabel* grayLabel = new QLabel(tr("Gy"));
    doseLayout->addWidget(doseNumberEdit);
    doseLayout->addWidget(grayLabel);
    layout->addRow(tr("Gesamtdosis"), doseLayout);

    regionLineEdit = new QLineEdit;
    layout->addRow(tr("Ort der Bestrahlung"), regionLineEdit);

    therapyLayout->addLayout(layout);

    connect(doseNumberEdit, SIGNAL(textEdited(QString)), this, SIGNAL(changed()));
    connect(regionLineEdit, SIGNAL(textEdited(QString)), this, SIGNAL(changed()));
}

TherapyElement* RTxEditWidget::applyToElement() const
{
    Radiotherapy* rtx = m_element->as<Radiotherapy>();
    rtx->dose = doseNumberEdit->text().toInt();
    rtx->location = regionLineEdit->text();
    return TherapyElementEditWidget::applyToElement();
}

void RTxEditWidget::setElement(TherapyElement* element)
{
    TherapyElementEditWidget::setElement(element);
    Radiotherapy* rtx = m_element->as<Radiotherapy>();
    doseNumberEdit->setText(rtx->dose ? QString::number(rtx->dose) : QString());
    regionLineEdit->setText(rtx->location);
}

ToxicityEditWidget::ToxicityEditWidget(QWidget* parent)
{
    therapyLabel->setText(tr("Komplikation"));

    QFormLayout* layout = new QFormLayout;
    typeLineEdit = new QLineEdit;
    layout->addRow(tr("Art / Toxizität"), typeLineEdit);

    gradeNumberEdit = new QLineEdit;
    QIntValidator* intValidator = new QIntValidator;
    gradeNumberEdit->setValidator(intValidator);
    layout->addRow(tr("Schweregrad"), gradeNumberEdit);

    therapyLayout->addLayout(layout);

    connect(typeLineEdit, SIGNAL(textEdited(QString)), this, SIGNAL(changed()));
    connect(gradeNumberEdit, SIGNAL(textEdited(QString)), this, SIGNAL(changed()));
}

TherapyElement* ToxicityEditWidget::applyToElement() const
{
    Toxicity* tox = m_element->as<Toxicity>();
    tox->grade = gradeNumberEdit->text().toInt();
    tox->description = typeLineEdit->text();
    return TherapyElementEditWidget::applyToElement();
}

void ToxicityEditWidget::setElement(TherapyElement* element)
{
    TherapyElementEditWidget::setElement(element);
    Toxicity* tox = m_element->as<Toxicity>();
    gradeNumberEdit->setText(tox->grade ? QString::number(tox->grade) : QString());
    typeLineEdit->setText(tox->description);
}

