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

#include "patiententerform.h"

// Qt includes

#include <QDate>
#include <QDateEdit>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QRadioButton>

class PatientEnterForm::PatientEnterFormPriv
{
public:
    PatientEnterFormPriv()
        : surnameEdit(0),
          firstNameEdit(0),
          dobEdit(0),
          maleButton(0),
          femaleButton(0)
    {
    }

    QLineEdit*    surnameEdit;
    QLineEdit*    firstNameEdit;
    QDateEdit*    dobEdit;
    QRadioButton* maleButton;
    QRadioButton* femaleButton;
};

PatientEnterForm::PatientEnterForm(QWidget *parent) :
    QWidget(parent),
    d(new PatientEnterFormPriv)
{
    QFormLayout* layout = new QFormLayout;
    d->surnameEdit   = new QLineEdit;
    d->firstNameEdit = new QLineEdit;
    d->dobEdit       = new QDateEdit;
    d->dobEdit->setDate(QDate(1956, 01, 01));
    QHBoxLayout* genderLayout = new QHBoxLayout;
    d->maleButton    = new QRadioButton(tr("Männlich"));
    d->femaleButton  = new QRadioButton(tr("Weiblich"));
    d->maleButton->setChecked(true);
    genderLayout->addWidget(d->maleButton);
    genderLayout->addWidget(d->femaleButton);
    genderLayout->addStretch();

    layout->addRow(tr("Vorname:"), d->firstNameEdit);
    layout->addRow(tr("Nachname:"), d->surnameEdit);
    layout->addRow(tr("Geburtsdatum:"), d->dobEdit);
    layout->addRow(tr("Geschlecht:"), genderLayout);
    setLayout(layout);

    connect(d->surnameEdit, SIGNAL(textEdited(QString)),
            this, SLOT(nameEdited()));
    connect(d->firstNameEdit, SIGNAL(textEdited(QString)),
            this, SLOT(nameEdited()));
}

PatientEnterForm::~PatientEnterForm()
{
    delete d;
}

Patient PatientEnterForm::currentPatient() const
{
    Patient p;
    p.firstName   = d->firstNameEdit->text();
    p.surname     = d->surnameEdit->text();
    p.dateOfBirth = d->dobEdit->date();
    p.gender      = d->maleButton->isChecked() ? Patient::Male : Patient::Female;
    return p;
}

void PatientEnterForm::newPatient()
{
    setValues(Patient());
    d->firstNameEdit->setFocus();
}

void PatientEnterForm::setValues(const Patient& patient)
{
    d->firstNameEdit->setText(patient.firstName);
    d->surnameEdit->setText(patient.surname);
    d->dobEdit->setDate(patient.dateOfBirth);
    d->maleButton->setChecked(patient.gender == Patient::Male || patient.gender == Patient::UnknownGender);
    d->femaleButton->setChecked(patient.gender == Patient::Female);
}

bool PatientEnterForm::isValid() const
{
    Patient p = currentPatient();
    return !(p.firstName.isEmpty() || p.surname.isEmpty() || p.dateOfBirth >= QDate::currentDate());
}

void PatientEnterForm::returnPressed()
{
    if (!isValid())
    {
        QMessageBox::warning(this, tr("Ungültige Daten"),
                             tr("Die Patientendaten sind nicht vollständig oder nicht plausibel"));
        return;
    }
    emit editingFinished(currentPatient());
}

void PatientEnterForm::keyPressEvent(QKeyEvent* e)
{
    switch (e->key())
    {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        returnPressed();
        return;
    }
    QWidget::keyPressEvent(e);
}

void PatientEnterForm::nameEdited()
{
    if (d->firstNameEdit->text().isEmpty() || d->surnameEdit->text().isEmpty())
    {
        return;
    }
    emit nameEdited(currentPatient());
}
