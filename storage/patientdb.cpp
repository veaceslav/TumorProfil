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

#include "patientdb.h"

// Qt includes

#include <QDebug>

// Local includes

#include "databasecorebackend.h"
#include "property.h"

class PatientDB::PatientDBPriv
{
public:
    PatientDBPriv()
        : db(0)
    {
    }

    DatabaseCoreBackend* db;

    inline QString tableName(PatientDB::PropertyType e)
    {
        switch (e)
        {
        case PatientDB::PatientProperties:
            return "PatientProperties";
        case PatientDB::PathologyProperties:
            return "PathologyProperties";
        }
        return QString();
    }

    inline QString idName(PatientDB::PropertyType e)
    {
        switch (e)
        {
        case PatientDB::PatientProperties:
            return "patientid";
        case PatientDB::PathologyProperties:
            return "pathologyid";
        }
        return QString();
    }
};

PatientDB::PatientDB(DatabaseCoreBackend* db)
    : d(new PatientDBPriv)
{
    d->db = db;
}

PatientDB::~PatientDB()
{
    delete d;
}

void PatientDB::setSetting(const QString& keyword, const QString& value)
{
    d->db->execSql( QString("REPLACE into Settings VALUES (?,?);"),
                    keyword, value );
}

QString PatientDB::setting(const QString& keyword)
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT value FROM Settings "
                            "WHERE keyword=?;"),
                    keyword, &values );

    if (values.isEmpty())
    {
        return QString();
    }
    else
    {
        return values.first().toString();
    }
}

int PatientDB::addPatient(const Patient& p)
{
    QVariant id;
    d->db->execSql("INSERT INTO Patients (firstName, surname, dateOfBirth, gender) "
                   "VALUES (?, ?, ?, ?)",
                   p.firstName, p.surname, p.dateOfBirth.toString(Qt::ISODate), p.gender, 0, &id);

    return id.toInt();
}

void PatientDB::updatePatient(const Patient& p)
{
    d->db->execSql("UPDATE Patients SET firstName=?, surname=?, dateOfBirth=?, gender=? WHERE id=?;",
                    QVariantList() << p.firstName << p.surname
                                   << p.dateOfBirth.toString(Qt::ISODate) << p.gender << p.id);
}

void PatientDB::deletePatient(int id)
{
    // Triggers do the rest
    d->db->execSql("DELETE FROM Patients WHERE id=?", id);
}

QList<Patient> PatientDB::findPatients(const Patient& p)
{
    QList<QVariant> values;

    QString sql = "SELECT id, firstName, surname, dateOfBirth, gender FROM Patients ";
    QString whereClause;
    QVariantList boundValues;
    if (!p.firstName.isNull())
    {
        whereClause += "firstName = ? AND ";
        boundValues << p.firstName;
    }
    if (!p.surname.isNull())
    {
        whereClause += "surname = ? AND ";
        boundValues << p.surname;
    }
    if (p.gender != Patient::UnknownGender)
    {
        whereClause += "gender = ? AND ";
        boundValues << p.gender;
    }
    if (!p.dateOfBirth.isNull())
    {
        whereClause += "firstName = ? AND ";
        boundValues << p.dateOfBirth.toString(Qt::ISODate);
    }

    if (!whereClause.isEmpty())
    {
        sql += "WHERE (" + whereClause + ");";
    }

    d->db->execSql(sql, boundValues, &values);

    QList<Patient> patients;
    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        Patient p;

        p.id          = it->toInt();
        ++it;
        p.firstName   = it->toString();
        ++it;
        p.surname     = it->toString();
        ++it;
        p.dateOfBirth = QDate::fromString(it->toString(), Qt::ISODate);
        ++it;
        p.gender      = (Patient::Gender)it->toInt();
        ++it;

        patients << p;
    }

    return patients;
}

int PatientDB::addDisease(int patientId, const Disease& dis)
{
    QVariant id;
    d->db->execSql("INSERT INTO Diseases (patientId, initialDiagnosis, cTNM, pTNM) "
                   "VALUES (?, ?, ?, ?)",
                   patientId, dis.initialDiagnosis.toString(Qt::ISODate),
                   dis.initialTNM.cTNM(), dis.initialTNM.pTNM(), 0, &id);
    return id.toInt();
}

void PatientDB::updateDisease(const Disease& dis)
{
    d->db->execSql("UPDATE Diseases SET initialDiagnosis=?, cTNM=?, pTNM=? WHERE id=?;",
                   dis.initialDiagnosis.toString(Qt::ISODate),
                   dis.initialTNM.cTNM(), dis.initialTNM.pTNM(), dis.id);
}

QList<Disease> PatientDB::findDiseases(int patientId)
{
    QList<QVariant> values;

    d->db->execSql("SELECT id, initialDiagnosis, cTNM, pTNM FROM Diseases WHERE patientId = ?;",
                   patientId, &values);

    QList<Disease> diseases;
    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        Disease d;

        d.id        = it->toInt();
        ++it;
        d.initialDiagnosis = QDate::fromString(it->toString(), Qt::ISODate);
        ++it;
        d.initialTNM.addTNM(it->toString()); // cTNM string
        ++it;
        d.initialTNM.addTNM(it->toString()); // pTNM string
        ++it;

        diseases << d;
    }

    return diseases;
}

int PatientDB::addPathology(int diseaseId, const Pathology& path)
{
    QVariant id;
    d->db->execSql("INSERT INTO Pathologies (diseaseId, entity, sampleOrigin) "
                   "VALUES (?, ?, ?)",
                   diseaseId, path.entity, path.sampleOrigin, 0, &id);
    return id.toInt();
}

void PatientDB::updatePathology(const Pathology& path)
{
    d->db->execSql("UPDATE Pathologies SET entity=?, sampleOrigin=? WHERE id=?;",
                   path.entity, path.sampleOrigin, path.id);
}

QList<Pathology> PatientDB::findPathologies(int diseaseId)
{
    QList<QVariant> values;

    d->db->execSql("SELECT id, entity, sampleOrigin FROM Pathologies WHERE diseaseId = ?;",
                   diseaseId, &values);

    QList<Pathology> pathologies;
    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        Pathology p;

        p.id           = it->toInt();
        ++it;
        p.entity       = (Pathology::Entity)it->toInt();
        ++it;
        p.sampleOrigin = (Pathology::SampleOrigin)it->toInt();
        ++it;

        pathologies << p;
    }

    return pathologies;
}

QList<Property> PatientDB::properties(PropertyType e, int id)
{
    QList<QVariant> values;

    d->db->execSql( "SELECT property, value, detail FROM " + d->tableName(e) +
                    " WHERE " + d->idName(e) + "=?;",
                    id, &values );

    QList<Property> properties;

    if (values.isEmpty())
    {
        return properties;
    }

    for (QList<QVariant>::const_iterator it = values.constBegin(); it != values.constEnd();)
    {
        Property property;

        property.property = (*it).toString();
        ++it;
        property.value    = (*it).toString();
        ++it;
        property.detail   = (*it).toString();
        ++it;

        properties << property;
    }

    return properties;
}

void PatientDB::addProperty(PropertyType e, int id, const QString& property,
                            const QString& value, const QString& detail)
{
    d->db->execSql("INSERT INTO " + d->tableName(e) +
                   " (" + d->idName(e) + ", property, value, detail) VALUES(?, ?, ?, ?);",
                   id, property, value, detail);
}

void PatientDB::removeProperties(PropertyType e, int id, const QString& property, const QString& value)
{
    if (property.isNull())
    {
        d->db->execSql("DELETE FROM " + d->tableName(e) + " WHERE " +
                       d->idName(e) + "=?;",
                       id);
    }
    else if (value.isNull())
    {
        d->db->execSql("DELETE FROM " + d->tableName(e) + " WHERE " +
                       d->idName(e) + "=? AND property=?;",
                       id, property);
    }
    else
    {
        d->db->execSql("DELETE FROM " + d->tableName(e) + " WHERE " +
                       d->idName(e) + "=? AND property=? AND value=?;",
                       id, property, value);
    }
}

