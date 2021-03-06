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

#ifndef PATIENT_H
#define PATIENT_H

// Qt includes

#include <QList>
#include <QDate>
#include <QMetaType>
#include <QSharedPointer>

// Local includes

#include "property.h"
#include "disease.h"

class Patient
{
public:
    Patient();
    Patient(const Patient& copy);
    ~Patient();

    enum Gender
    {
        UnknownGender,
        Male,
        Female
    };

    typedef QSharedPointer<Patient> Ptr;

public:

    /** Needs at least names and a date of birth */
    bool isValid() const;
    bool hasDisease() const;
    Disease& firstDisease();
    const Disease& firstDisease() const;
    bool hasPathology() const;

    /**
     * @brief encrypt - Method used for encrypting patient data
     *                  before storing in database
     * @return          true if encryption is enabled and it was successful
     */
    bool encrypt();

    /**
     * @brief decrypt - Method to decrypt patient data after retrieving from storage
     * @return        - true if decryption is enabled and no errors occured
     */
    bool decrypt();

    QString             firstName;
    QString             surname;
    QDate               dateOfBirth;
    Gender              gender; // true = male; false = female
    PropertyList        patientProperties;
    QString             encryptedDateOfBirth;

    QList<Disease>      diseases;

    int                 id;

    bool operator==(const Patient& other) const;

    /**
      Sets the attributes firstName, surname, dateOfBirth and gender
      from the given Patient. Does not touch properties, diseases or id.
      */
    void setPatientData(const Patient& p);

    void defaultDateOfBirth(QString date);

};

Q_DECLARE_METATYPE(Patient::Ptr)

#endif // PATIENT_H
