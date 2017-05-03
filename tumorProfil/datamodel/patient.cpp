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
#include "authentication//userinformation.h"
#include "TumorUsers/aesutils.h"
#include "constants.h"

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

bool Patient::encrypt()
{
    if(!UserInformation::instance()->isEncryptionEnabled())
    {
        encryptedDateOfBirth = dateOfBirth.toString(Qt::ISODate);
        return false;
    }

    // The user is not logged in, but we allow to browse anonymous
    if(!UserInformation::instance()->isLoggedIn()){
        encryptedDateOfBirth = dateOfBirth.toString(Qt::ISODate);
        return true;
    }

    UserInformation* user = UserInformation::instance();

    qDebug() << !(firstName.isEmpty())  << user->hasKey(SQL_PATIENT_NAME);
    if(!(firstName.isEmpty()) && user->hasKey(SQL_PATIENT_NAME) )
    {
        qDebug() << "Encrypting first name";
        this->firstName = AesUtils::encrypt(this->firstName, user->retrieveKey(SQL_PATIENT_NAME));
    }

    if(!(surname.isEmpty()) && user->hasKey(SQL_PATIENT_SURNAME))
    {
        qDebug() << "Encrypting surname";
        this->surname = AesUtils::encrypt(this->surname, user->retrieveKey(SQL_PATIENT_SURNAME));
    }

    if(dateOfBirth.isValid() && user->hasKey(SQL_PATIENT_DATEOFBIRTH))
    {
        qDebug() << "Encrypting date";
        this->encryptedDateOfBirth = AesUtils::encrypt(this->dateOfBirth.toString(Qt::ISODate),
                                                       user->retrieveKey(SQL_PATIENT_DATEOFBIRTH));
    }
    else
    {
        this->encryptedDateOfBirth = this->dateOfBirth.toString(Qt::ISODate);
    }

    return true;
}

bool Patient::decrypt()
{
    if(!UserInformation::instance()->isEncryptionEnabled())
    {
        // We still need to get the date from encrypted date
        defaultDateOfBirth(encryptedDateOfBirth);
        return false;
    }

    // The user is not logged in, but we allow to browse anonymous
    // We try to get the date and if it is encrypted, then set a default
    if(!UserInformation::instance()->isLoggedIn())
    {
        defaultDateOfBirth(encryptedDateOfBirth);
        return true;
    }

    UserInformation* user = UserInformation::instance();

    if(!this->firstName.isEmpty() && user->hasKey(SQL_PATIENT_NAME))
    {
        this->firstName = AesUtils::decrypt(this->firstName, user->retrieveKey(SQL_PATIENT_NAME));
    }

    if(!this->surname.isEmpty() && user->hasKey(SQL_PATIENT_SURNAME))
    {
        this->surname = AesUtils::decrypt(this->surname, user->retrieveKey(SQL_PATIENT_SURNAME));
    }

    if(!this->encryptedDateOfBirth.isEmpty() && user->hasKey(SQL_PATIENT_DATEOFBIRTH))
    {
        defaultDateOfBirth(AesUtils::decrypt(this->encryptedDateOfBirth,
                                         user->retrieveKey(SQL_PATIENT_DATEOFBIRTH)));

    }
    else
    {
        defaultDateOfBirth(QString());
    }

    return true;
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

void Patient::defaultDateOfBirth(QString date)
{
    dateOfBirth = QDate::fromString(date, Qt::ISODate);
    if(!dateOfBirth.isValid())
    {
        dateOfBirth = QDate::fromString("1900-01-01", Qt::ISODate);
    }
}

bool Patient::operator==(const Patient& other) const
{
    return firstName   == other.firstName
        && surname     == other.surname
        && gender      == other.gender
        && dateOfBirth == other.dateOfBirth;
}
