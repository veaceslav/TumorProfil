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

#include "patientdisplay.h"

// Qt includes

#include <QHBoxLayout>
#include <QLabel>

class PatientDisplay::PatientDisplayPriv
{
public:
    PatientDisplayPriv()
        : name(0),
          gender(0),
          dateOfBirth(0)
    {
    }

    QLabel *name;
    QLabel *gender;
    QLabel *dateOfBirth;
};

PatientDisplay::PatientDisplay(QWidget *parent)
    : QWidget(parent),
      d(new PatientDisplayPriv)
{
    QHBoxLayout* layout = new QHBoxLayout;

    d->name        = new QLabel;
    d->gender      = new QLabel;
    d->dateOfBirth = new QLabel;

    layout->addWidget(d->name);
    layout->addWidget(d->gender);
    layout->addWidget(d->dateOfBirth);

    setLayout(layout);

    QFont f = font();
    f.setBold(true);
    setFont(f);
}

PatientDisplay::~PatientDisplay()
{
    delete d;
}

void PatientDisplay::setShowGender(bool show)
{
    d->gender->setVisible(show);
}

void PatientDisplay::setPatient(const Patient::Ptr &p)
{
    if (p)
    {
        setPatient(*p);
    }
    else
    {
        clear();
    }
}

void PatientDisplay::setPatient(const Patient& patient)
{
    if (patient.isValid())
    {
        //: Name display, Surname, first name
        d->name->setText(tr("%1, %2").arg(patient.surname).arg(patient.firstName));
        d->gender->setText(patient.gender == Patient::Male ? tr("Männlich") : tr("Weiblich"));
        d->dateOfBirth->setText(patient.dateOfBirth.toString("dd.MM.yyyy"));
    }
    else
    {
        clear();
    }
}

void PatientDisplay::clear()
{
    d->name->setText(QString()); //(tr("(Kein Patient)"));
    d->gender->setText(QString());
    d->dateOfBirth->setText(QString());
}
