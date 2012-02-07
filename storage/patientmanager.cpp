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

    void moreSchemaUpdateSteps(int numberOfSteps)
    {
        qApp->setOverrideCursor(Qt::WaitCursor);
    }

    void schemaUpdateProgress(const QString& message, int numberOfSteps)
    {
    }

    void finishedSchemaUpdate(UpdateResult result)
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

bool PatientManager::initialize()
{
    DatabaseParameters params = DatabaseParameters::parametersForSQLite("test.db");
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
    // TODO: check for duplicate
    Patient::Ptr p = createPatient(values);
    storeData(p);
    emit patientAdded(d->patients.size()-1, p);
    return p;
}

void PatientManager::updateData(const Patient::Ptr& patient)
{
    if (!patient)
    {
        return;
    }
    storeData(patient);
    // we dont check for actual modification here
    emit patientDataChanged(patient);
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


void PatientManager::storeData(const Patient::Ptr& patient)
{
    if (!patient || !patient->id)
    {
        qWarning() << "Invalid patient given to storeData";
    }

    DatabaseOperationGroup group;
    group.setMaximumTime(200);

    DatabaseAccess().db()->updatePatient(*patient);
    DatabaseAccess().db()->removeProperties(PatientDB::PatientProperties, patient->id);
    foreach (const Property& property, patient->patientProperties)
    {
        DatabaseAccess().db()->addProperty(PatientDB::PatientProperties, patient->id,
                                           property.property, property.value, property.detail);
    }

    for (int i=0; i<patient->diseases.size(); ++i)
    {
        Disease& disease = patient->diseases[i];
        if (disease.id)
        {
            DatabaseAccess().db()->updateDisease(disease);
        }
        else
        {
            disease.id = DatabaseAccess().db()->addDisease(patient->id, disease);
        }

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
        disease.pathologies = DatabaseAccess().db()->findPathologies(disease.id);
        for (int u=0; u<disease.pathologies.size(); ++u)
        {
            Pathology& pathology = disease.pathologies[u];
            pathology.properties = DatabaseAccess().db()->properties(PatientDB::PathologyProperties, pathology.id);
        }
    }
}

