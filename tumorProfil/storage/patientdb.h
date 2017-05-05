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

#ifndef PATIENTDB_H
#define PATIENTDB_H

// Qt includes

#include <QString>
#include <QVariant>

// Local includes

#include "event.h"
#include "patient.h"
#include "property.h"

class DatabaseCoreBackend;

class PatientDB
{
public:

    PatientDB(DatabaseCoreBackend* db);
    ~PatientDB();

    void setSetting(const QString& keyword, const QString& value);
    QString setting(const QString& keyword);

    int addPatient(const Patient& p);
    void updatePatient(const Patient& p);
    /** Finds patients matching criteria defined by p.
        Any null field of p will be treated as a wildcard.
        A null patient will retrieve all patients.
        Please note that properties are ignored for this purpose.
        */
    QList<Patient> findPatients(const Patient& p = Patient());

    int addDisease(int patientId, const Disease& d);
    void updateDisease(const Disease& dis);
    void deletePatient(int id);
    QList<Disease> findDiseases(int patientId);

    int addPathology(int diseaseId, const Pathology& path);
    void updatePathology(const Pathology& path);
    QList<Pathology> findPathologies(int diseaseId);

    enum PropertyType
    {
        PatientProperties,
        DiseaseProperties,
        PathologyProperties
    };

    QList<Property> properties(PropertyType e, int id);
    void addProperty(PropertyType e, int id, const QString& property,
                     const QString& value, const QString& detail);
    void removeProperties(PropertyType e, int id,
                          const QString& property = QString(),
                          const QString& value = QString());

    void replaceEvents(int diseaseId, const QList<Event> events);
    QList<Event> findEvents(int diseaseId);

private:


    class PatientDBPriv;
    PatientDBPriv* const d;
};

#endif // PATIENTDB_H
