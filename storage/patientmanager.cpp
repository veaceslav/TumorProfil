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

// Qt includes

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>

// Local includes

#include "databaseaccess.h"
#include "databaseinitializationobserver.h"
#include "databaseoperationgroup.h"
#include "databaseparameters.h"
#include "patient.h"
#include "patientdb.h"
#include "patientmanager.h"

class PatientManager::PatientManagerPriv
{
public:
    PatientManagerPriv()
    {

    }

    QList<Patient::Ptr>      patients;
    QHash<int, int>          patientIdHash;
};

class DefaultInitializationObserver : public InitializationObserver
{
public:

    DefaultInitializationObserver() : success(true) {}
    bool success;
    virtual bool continueQuery()
    {
        return true;
    }

    void moreSchemaUpdateSteps(int)
    {
        qApp->setOverrideCursor(Qt::WaitCursor);
    }

    void schemaUpdateProgress(const QString&, int)
    {
    }

    void finishedSchemaUpdate(UpdateResult)
    {
        qApp->restoreOverrideCursor();
    }

    void error(const QString& errorMessage)
    {
        success = false;
        QMessageBox::critical(0, QObject::tr("Datenbankproblem"),
                              QObject::tr("Kritischer Datenbankfehler: %1").arg(errorMessage));
    }
};

PatientManager::PatientManager(QObject *parent) :
    QObject(parent),
    d(new PatientManagerPriv)
{
}

PatientManager::~PatientManager()
{
    delete d;
}

class PatientManagerCreator { public: PatientManager object; };
Q_GLOBAL_STATIC(PatientManagerCreator, creator)

PatientManager* PatientManager::instance()
{
    return &creator()->object;
}

bool PatientManager::initialize(const DatabaseParameters& params)
{
    DatabaseAccess::setParameters(params);

    DefaultInitializationObserver observer;
    DatabaseAccess::checkReadyForUse(&observer);
    return observer.success;
}

void PatientManager::readDatabase()
{
    QList<Patient> patients = DatabaseAccess().db()->findPatients();
    QHash<int, int> oldIds = d->patientIdHash;
    QList<Patient::Ptr> newPatientList;
    foreach (const Patient& data, patients)
    {
        int index = d->patientIdHash.value(data.id, -1);
        oldIds.remove(data.id);
        if (index == -1)
        {
            Patient::Ptr p = createPatient(data);
            loadData(p);
            emit patientAdded(d->patients.size()-1, p);
        }
        else
        {
            loadData(d->patients[index]);
        }
    }
    foreach (int index, oldIds)
    {
        cleanUpPatient(index);
    }
}

Patient::Ptr PatientManager::addPatient(const Patient& values)
{
    if (!values.isValid())
    {
        qDebug() << "Refusing to add invalid patient data.";
        return Patient::Ptr();
    }

    // check for duplicate
    QList<Patient::Ptr> ps = findPatients(values);
    if (!ps.isEmpty())
    {
        qDebug() << "Attempt to add duplicate patient";
        return ps.first();
    }

    Patient::Ptr p = createPatient(values);
    storeData(p, ChangedPatientMetadata);
    emit patientAdded(d->patients.size()-1, p);
    return p;
}

void PatientManager::updateData(const Patient::Ptr& patient, ChangeFlags flags)
{
    if (!patient)
    {
        return;
    }
    storeData(patient, flags);
    // we dont check for actual modification here
    emit patientDataChanged(patient, flags);
}

Patient::Ptr PatientManager::createPatient(const Patient& values)
{
    Patient::Ptr ptr(new Patient(values));
    if (!ptr->id)
    {
        ptr->id = DatabaseAccess().db()->addPatient(values);
    }
    d->patients << ptr;
    d->patientIdHash[ptr->id] = d->patients.size() - 1;
    return ptr;
}

void PatientManager::removePatient(const Patient::Ptr& patient)
{
    if (!patient || !patient->id)
    {
        return;
    }
    int index = d->patientIdHash.value(patient->id, -1);
    if (index == -1)
    {
        return;
    }

    DatabaseAccess().db()->deletePatient(patient->id);
    cleanUpPatient(index);
}


Patient::Ptr PatientManager::patientForId(int patientId) const
{
    int index = d->patientIdHash.value(patientId, -1);
    if (index == -1)
    {
        return Patient::Ptr();
    }
    return d->patients[index];
}

int PatientManager::indexOfPatient(const Patient::Ptr& ptr) const
{
    if (!ptr)
    {
        return -1;
    }
    return d->patientIdHash.value(ptr->id, -1);
}

static bool matches(const QString& value, const QString& match)
{
    if (value.isNull())
    {
        return true;
    }
    if (match.endsWith("*"))
    {
        QString s1 = match.left(match.size()-1);
        QString s2 = value.left(s1.size());
        return value.compare(s1, s2, Qt::CaseInsensitive) == 0;
    }
    return value.compare(value, match, Qt::CaseInsensitive) == 0;
}

static bool matches(const QDate& value, const QDate& match)
{
    if (match.isNull())
    {
        return true;
    }
    return value == match;
}

static bool matches(Patient::Gender value, Patient::Gender match)
{
    if (match == Patient::UnknownGender)
    {
        return true;
    }
    return value == match;
}

QList<Patient::Ptr> PatientManager::findPatients(const QString& surname, const QString& firstName,
                                                 const QDate& dob, Patient::Gender gender)
{
    Patient p;
    p.surname = surname;
    p.firstName = firstName;
    p.dateOfBirth = dob;
    p.gender = gender;
    return findPatients(p);
}

QList<Patient::Ptr> PatientManager::findPatients(const Patient& match)
{
    QList<Patient::Ptr> ps;
    foreach (const Patient::Ptr& p, d->patients)
    {
        if (matches(p->surname, match.surname)
                && matches(p->firstName, match.firstName)
                && matches(p->dateOfBirth, match.dateOfBirth)
                && matches(p->gender, match.gender))
        {
            ps << p;
        }
    }
    return ps;
}


Patient::Ptr PatientManager::patient(int index) const
{
    return d->patients[index];
}

int PatientManager::patientId(int index) const
{
    return d->patients[index]->id;
}

const QList<Patient::Ptr> PatientManager::patients() const
{
    return d->patients;
}

int PatientManager::numberOfPatients() const
{
    return d->patients.size();
}

void PatientManager::cleanUpPatient(int index)
{
    Patient::Ptr p = d->patients[index];
    emit patientAboutToBeRemoved(index, p);

    // update idHash - which points to indexes of d->patients, and these change now!
    QHash<int, int>::iterator it;
    for (it = d->patientIdHash.begin(); it != d->patientIdHash.end(); )
    {
        if (it.value() == index)
        {
            it = d->patientIdHash.erase(it);
            continue;
        }
        else if (it.value() > index)
        {
                it.value()--;
        }
        ++it;
    }
    d->patients.removeAt(index);

    emit patientRemoved(p);
}


void PatientManager::storeData(const Patient::Ptr& patient, ChangeFlags flags)
{
    if (!patient || !patient->id)
    {
        qWarning() << "Invalid patient given to storeData";
    }

    DatabaseOperationGroup group;
    group.setMaximumTime(200);

    if (flags & ChangedPatientMetadata)
    {
        DatabaseAccess().db()->updatePatient(*patient);
    }

    if (flags & ChangedDiseaseProperties)
    {
        DatabaseAccess().db()->removeProperties(PatientDB::PatientProperties, patient->id);
        foreach (const Property& property, patient->patientProperties)
        {
            //qDebug() << "Patient property" << property.property << property.value;
            DatabaseAccess().db()->addProperty(PatientDB::PatientProperties, patient->id,
                                               property.property, property.value, property.detail);
        }
    }

    for (int i=0; i<patient->diseases.size(); ++i)
    {
        //qDebug() << "Storing disease";
        Disease& disease = patient->diseases[i];
        if (disease.id)
        {
            if (flags & ChangedDiseaseMetadata)
            {
                DatabaseAccess().db()->updateDisease(disease);
            }
        }
        else
        {
            disease.id = DatabaseAccess().db()->addDisease(patient->id, disease);
        }

        if (flags & ChangedDiseaseProperties)
        {
            //qDebug() << "Storing disease properties";
            DatabaseAccess().db()->removeProperties(PatientDB::DiseaseProperties, disease.id);
            foreach (const Property& property, disease.diseaseProperties)
            {
                //qDebug() << "Adding property" << property.property << property.value;
                DatabaseAccess().db()->addProperty(PatientDB::DiseaseProperties, disease.id,
                                                   property.property, property.value, property.detail);
            }
        }


        if (flags & ChangedPathologyData)
        {
            for (int u=0; u<disease.pathologies.size(); ++u)
            {
                Pathology& pathology = disease.pathologies[u];
                if (pathology.id)
                {
                    DatabaseAccess().db()->updatePathology(pathology);
                }
                else
                {
                    pathology.id = DatabaseAccess().db()->addPathology(disease.id, pathology);
                }

                DatabaseAccess().db()->removeProperties(PatientDB::PathologyProperties, pathology.id);
                foreach (const Property& property, pathology.properties)
                {
                    DatabaseAccess().db()->addProperty(PatientDB::PathologyProperties, pathology.id,
                                                       property.property, property.value, property.detail);
                }
            }
        }
    }
}

void PatientManager::loadData(const Patient::Ptr& p)
{
    if (!p || !p->id)
    {
        qWarning() << "Invalid patient given to loadData";
    }

    p->patientProperties = DatabaseAccess().db()->properties(PatientDB::PatientProperties, p->id);
    p->diseases = DatabaseAccess().db()->findDiseases(p->id);
    for (int i=0; i<p->diseases.size(); ++i)
    {
        Disease& disease = p->diseases[i];
        disease.diseaseProperties = DatabaseAccess().db()->properties(PatientDB::DiseaseProperties, disease.id);
        disease.pathologies = DatabaseAccess().db()->findPathologies(disease.id);
        for (int u=0; u<disease.pathologies.size(); ++u)
        {
            Pathology& pathology = disease.pathologies[u];
            pathology.properties = DatabaseAccess().db()->properties(PatientDB::PathologyProperties, pathology.id);
        }
    }
}

void PatientManager::historySecurityCopy(const Patient::Ptr& p, const QString& type, const QString& value)
{
    DatabaseParameters params = DatabaseAccess::parameters();
    QFileInfo file(params.databaseName);
    QDir dir = file.dir();
    if (!dir.exists("Sicherung"))
    {
        dir.mkdir("Sicherung");
    }
    dir.cd("Sicherung");
    QString fileName = p->surname + '-' + p->firstName + '-' + p->dateOfBirth.toString(Qt::ISODate)
            + '-' + type + '-' + QDateTime::currentDateTime().toString(Qt::ISODate);
    fileName.remove(":");
    QString filePath = dir.filePath(fileName);
    QFile f(filePath);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        f.write(value.toUtf8());
    }
    else
    {
        qDebug() << "Failed to open" << filePath;
    }
}
