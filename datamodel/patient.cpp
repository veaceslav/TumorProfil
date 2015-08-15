/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 2012-01-22
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

#include <QDebug>

#include "patient.h"

Patient::Patient()
    : gender(UnknownGender),
      id(0)
{
}

Patient::Patient(const Patient &copy)
{
    setPatientData(copy);
    this->id = copy.id;
}

Patient::~Patient()
{
}

bool Patient::isValid() const
{
    return !firstName.isEmpty() &&
           !surname.isEmpty() &&
            gender != UnknownGender &&
            dateOfBirth.isValid();
}

bool Patient::hasDisease() const
{
    return !diseases.isEmpty();
}
Disease& Patient::firstDisease()
{
    if (diseases.isEmpty())
    {
        qDebug() << "Expect crash";
    }
    return diseases.first();
}

const Disease& Patient::firstDisease() const
{
    return diseases.first();
}

bool Patient::hasPathology() const
{
    if (hasDisease())
    {
        return firstDisease().hasPathology();
    }
    return false;
}

/*const Pathology& Patient::firstPathology() const
{
    return firstDisease().firstPathology();
}*/

void Patient::setPatientData(const Patient& p)
{
    firstName   = p.firstName;
    surname     = p.surname;
    gender      = p.gender;
    dateOfBirth = p.dateOfBirth;
}

bool Patient::operator==(const Patient& other) const
{
    return firstName   == other.firstName
        && surname     == other.surname
        && gender      == other.gender
        && dateOfBirth == other.dateOfBirth;
}
