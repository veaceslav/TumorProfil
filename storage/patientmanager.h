/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 02.02.2012
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

#ifndef PATIENTMANAGER_H
#define PATIENTMANAGER_H

// Qt includes

#include <QFlags>
#include <QObject>

// Local includes

#include "patient.h"

class DatabaseParameters;

class PatientManager : public QObject
{
    Q_OBJECT

public:

    enum ChangeFlag
    {
        ChangedNothing           = 0,
        ChangedPathologyData     = 1 << 0,
        ChangedDiseaseProperties = 1 << 1,
        ChangedDiseaseMetadata   = 1 << 2,
        ChangedPatientProperties = 1 << 3,
        ChangedPatientMetadata   = 1 << 4,

        ChangedDiseaseHistory    = ChangedDiseaseProperties,
        ChangedAll               = ChangedPathologyData | ChangedDiseaseProperties
                                  | ChangedDiseaseMetadata | ChangedPatientProperties
                                  | ChangedPatientMetadata
    };
    Q_DECLARE_FLAGS(ChangeFlags, ChangeFlag)

    static PatientManager* instance();

    bool initialize(const DatabaseParameters& params);

    void readDatabase();

    Patient::Ptr addPatient(const Patient& values);
    void updateData(const Patient::Ptr& patient, ChangeFlags flags);
    void removePatient(const Patient::Ptr& patient);

    Patient::Ptr patient(int index) const;
    int patientId(int index) const;
    const QList<Patient::Ptr> patients() const;
    int numberOfPatients() const;
    Patient::Ptr patientForId(int patientId) const;
    int indexOfPatient(const Patient::Ptr& ptr) const;

    QList<Patient::Ptr> findPatients(const Patient& p);
    QList<Patient::Ptr> findPatients(const QString& surname,
                                     const QString& firstName = QString(),
                                     const QDate& dob = QDate(),
                                     Patient::Gender gender = Patient::UnknownGender);

    void historySecurityCopy(const Patient::Ptr& p, const QString& type, const QString& value);
    void mergeDatabase(const DatabaseParameters& otherDb);

signals:

    void patientAdded(int index, const Patient::Ptr& patient);
    void patientDataChanged(const Patient::Ptr& patient, int flags);
    void patientAboutToBeRemoved(int index, const Patient::Ptr& patient);
    void patientRemoved(const Patient::Ptr& patient);

public slots:

protected:

    void loadData(const Patient::Ptr& patient);
    Patient::Ptr createPatient(const Patient& values);
    void cleanUpPatient(int index);
    void storeData(const Patient::Ptr& patient, ChangeFlags flags);

private:

    PatientManager(QObject *parent = 0);
    ~PatientManager();
    friend class PatientManagerCreator;
    class PatientManagerPriv;
    PatientManagerPriv* const d;

};

Q_DECLARE_OPERATORS_FOR_FLAGS(PatientManager::ChangeFlags)

#endif // PATIENTMANAGER_H
