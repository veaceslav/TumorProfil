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
#include <QTextEdit>

// Local includes

#include "databaseaccess.h"
#include "databaseconstants.h"
#include "databasetransaction.h"
#include "databaseinitializationobserver.h"
#include "databaseoperationgroup.h"
#include "databaseparameters.h"
#include "diseasehistory.h"
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

bool PatientManager::initialize()
{
    DefaultInitializationObserver observer;
    DatabaseAccess::checkReadyForUse(&observer);
    return observer.success;
}

void PatientManager::readDatabase()
{
    QList<Patient> patients = DatabaseAccess().db()->findPatients();
    emit progressStarted(patients.size());
    QHash<int, int> oldIds = d->patientIdHash;
    QList<Patient::Ptr> newPatientList;
    foreach (Patient data, patients)
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
        emit progressValue(d->patients.size());
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

    if (flags & ChangedPatientProperties)
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
            DatabaseAccess().db()->removeProperties(PatientDB::DiseaseProperties, disease.id);
            foreach (const Property& property, disease.diseaseProperties)
            {
                DatabaseAccess().db()->addProperty(PatientDB::DiseaseProperties, disease.id,
                                                   property.property, property.value, property.detail);
            }
        }

        if (flags & ChangedDiseaseHistory)
        {
            QList<Event> events = disease.history.toEvents();
            DatabaseAccess().db()->replaceEvents(disease.id, events);
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
                foreach (const QString& text, pathology.reports)
                {
                    DatabaseAccess().db()->addProperty(PatientDB::PathologyProperties, pathology.id,
                                                       PathologyPropertyName::pathologyReportId(), text, QString());
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

    //NOTE: Keep in sync with mergeDatabase code below
    p->patientProperties = DatabaseAccess().db()->properties(PatientDB::PatientProperties, p->id);
    p->diseases = DatabaseAccess().db()->findDiseases(p->id);
    if (p->diseases.isEmpty())
    {
        qWarning() << "Patient" << p->firstName << p->surname << "has no disease in Database";
    }
    for (int i=0; i<p->diseases.size(); ++i)
    {
        Disease& disease = p->diseases[i];
        disease.diseaseProperties = DatabaseAccess().db()->properties(PatientDB::DiseaseProperties, disease.id);
        QList<Event> events = DatabaseAccess().db()->findEvents(disease.id);
        disease.history = DiseaseHistory::fromEvents(events);
        // MIGRATION: Load XML alternatively
        if (disease.history.isEmpty())
        {
            disease.history = disease.historyFromProperties();
        }
        disease.pathologies = DatabaseAccess().db()->findPathologies(disease.id);
        for (int u=0; u<disease.pathologies.size(); ++u)
        {
            Pathology& pathology = disease.pathologies[u];
            pathology.properties = DatabaseAccess().db()->properties(PatientDB::PathologyProperties, pathology.id);
            // Sort out pathology report properties to separate list
            for (PropertyList::iterator it = pathology.properties.begin(); it != pathology.properties.end(); )
            {
                if (it->property == PathologyPropertyName::pathologyReportId())
                {
                    pathology.reports += it->value;
                    it = pathology.properties.erase(it);
                }
                else
                {
                    ++it;
                }
            }
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

class QMessageBoxResize: public QMessageBox
{
public:
    QMessageBoxResize() {
        setMouseTracking(true);
        setSizeGripEnabled(true);
    }
    QMessageBoxResize( Icon icon, const QString & title, const QString & text, StandardButtons buttons = NoButton, QWidget * parent = 0, Qt::WindowFlags f = Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint )
        : QMessageBox(icon, title, text, buttons, parent, f)
    {
        setMouseTracking(true);
        setSizeGripEnabled(true);
    }
private:
    virtual bool event(QEvent *e) {
        bool res = QMessageBox::event(e);
        switch (e->type()) {
        case QEvent::MouseMove:
        case QEvent::MouseButtonPress:
            setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
            if (QWidget *textEdit = findChild<QTextEdit *>()) {
                textEdit->setMaximumHeight(QWIDGETSIZE_MAX);
            }
        default:break;
        }
        return res;
    }
};

static bool checkHistoryShouldBeReplaced(const Disease& currentD, const DiseaseHistory& otherHistory, const DiseaseHistory& currentHistory, const QString& patientIdentifierString, QStringList* mergeHints = 0)
{
    // trivial case
    if (currentHistory.isEmpty() && !otherHistory.isEmpty())
    {
        return true;
    }
    if (currentHistory == otherHistory)
    {
        return false;
    }

    // by last documentation date
    if (otherHistory.lastDocumentation().isValid())
    {
        if (currentHistory.lastDocumentation().isValid())
        {
            if (otherHistory.lastDocumentation() != currentHistory.lastDocumentation())
            {
                return otherHistory.lastDocumentation() > currentHistory.lastDocumentation();
            }
            else
            {
                if (otherHistory.lastValidation().isValid())
                {
                    if (!currentHistory.lastValidation().isValid() || otherHistory.lastValidation() > currentHistory.lastValidation())
                    {
                        return true;
                    }
                    else if (currentHistory.lastValidation().isValid() && otherHistory.lastValidation() < currentHistory.lastValidation())
                    {
                        return false;
                    }
                }

                // Here, we have differing histories with the same last-documentation date and no decision from last validation
                bool hint = false;

                if (currentD.entity() == Pathology::ColorectalAdeno)
                {
                    hint = true;
                }

                if (hint)
                {
                    if (mergeHints)
                    {
                        *mergeHints << "Positive decision to merge histories based on hint for " + patientIdentifierString;
                    }
                    return true;
                }
                if (mergeHints)
                {
                    *mergeHints << "Impossible decision to merge histories, not merged. Add Hint if needed: " + patientIdentifierString;
                }
                return false;
            }
        }
        else
        {
            // last documentation was added, this indicates improvement
            return true;
        }
    }

    const bool longer = (otherHistory.size() > currentHistory.size());
    const bool newer  = (otherHistory.end() > currentHistory.end());

    if (currentHistory.lastDocumentation().isValid())
    {
        if (longer)
        {
            if (mergeHints)
                *mergeHints << "Discarding merged history, which is longer, but has lastDocumentation not set, while current history set it, for " + patientIdentifierString;
        }
        return false;
    }
    else
    {
        // both have lastDocumentation not set.
        if (newer && longer)
        {
            return true;
        }
        else if (!newer && !longer)
        {
            return false;
        }
        else if (newer && !longer)
        {
            if (otherHistory.size() < currentHistory.size())
            {
                if (mergeHints)
                    *mergeHints << "Merging history which is newer, but lost size, for" + patientIdentifierString;
            }
            return true;
        }
        else //if (!newer && longer)
        {
            if (otherHistory.end() == currentHistory.end())
            {
                // is longer and has same end date
                return true;
            }
            else
            {
                if (mergeHints)
                    *mergeHints << "Discarding merged history, which is longer, but current history is newer, for" + patientIdentifierString;
                return false;
            }

        }
    }
}

void PatientManager::mergeDatabase(const DatabaseParameters& otherDb)
{
    DefaultInitializationObserver observer;
    DatabaseAccess* access = DatabaseAccess::createExternalPatientDBAccess(otherDb, &observer);
    if (!access)
    {
        qWarning() << "Failed to access other database" << otherDb;
    }

    QList<Patient> patients = access->db()->findPatients();
    for (int i=0; i<patients.size(); ++i)
    {
        Patient& p = patients[i];

        p.patientProperties = access->db()->properties(PatientDB::PatientProperties, p.id);
        p.diseases = access->db()->findDiseases(p.id);
        for (int i=0; i<p.diseases.size(); ++i)
        {
            Disease& disease = p.diseases[i];
            disease.diseaseProperties = access->db()->properties(PatientDB::DiseaseProperties, disease.id);
            disease.pathologies = access->db()->findPathologies(disease.id);
            for (int u=0; u<disease.pathologies.size(); ++u)
            {
                Pathology& pathology = disease.pathologies[u];
                pathology.properties = access->db()->properties(PatientDB::PathologyProperties, pathology.id);
            }
        }
    }
    delete access;
    qDebug() << "Currently" << d->patients.size() << "patients, new db" << patients.size();

    // Dry run
    QStringList mergeActions, mergeHints;
    foreach (const Patient& other, patients)
    {
        Patient::Ptr p;
        QList<Patient::Ptr> ps = findPatients(other);
        if (ps.isEmpty())
        {
            mergeActions << "Merge new Patient " + other.firstName + " " + other.surname + " "  + other.dateOfBirth.toString();
            continue;
        }
        else
        {
            p = ps.first();
            if (ps.size() > 1)
            {
                mergeActions << "Warning: Multiple patients found for" + other.firstName + other.surname + other.dateOfBirth.toString();
            }
        }

        QString patientIdentifierString = p->firstName + " " + p->surname + ", " + p->dateOfBirth.toString();

        ChangeFlags changed = ChangedNothing;
        if (p->patientProperties != other.patientProperties)
        {
            changed |= ChangedPatientProperties;
        }
        foreach (const Disease& otherD, other.diseases)
        {
            int diseaseIndex;
            for (diseaseIndex=0; diseaseIndex<p->diseases.size(); ++diseaseIndex)
            {
                if (p->diseases[diseaseIndex].entity() == otherD.entity())
                {
                    break;
                }
            }
            if (diseaseIndex == p->diseases.size())
            {
                if (p->diseases.size())
                {
                    mergeHints << "Second entity for " + patientIdentifierString + ", please check manually";
                }
                else
                {
                    mergeActions << "Adding new disease for " + patientIdentifierString;
                    changed |= ChangedDiseaseMetadata | ChangedDiseaseProperties | ChangedDiseaseHistory | ChangedPathologyData;
                    continue;
                }
            }
            else
            {
                Disease& d = p->diseases[diseaseIndex];
                // ! initialDiagnosis: Do not merge
                // ! initialTNM: Do not merge
                // ! diseaseProperties: Handle history
                if (d.diseaseProperties != otherD.diseaseProperties)
                {
                    DiseaseHistory h = d.history;
                    DiseaseHistory otherH = otherD.history;
                    if (checkHistoryShouldBeReplaced(d, otherH, h, patientIdentifierString, &mergeHints))
                    {
                        QString entityString; if (d.entity() == Pathology::ColorectalAdeno) entityString="CRC"; if (d.entity() == Pathology::PulmonaryAdeno) entityString="ADC";
                        mergeActions << entityString + " History of "+ patientIdentifierString + " will be replaced";
                        changed |= ChangedDiseaseHistory;
                    }
                }
                foreach (const Pathology& otherP, otherD.pathologies)
                {
                    if (!d.hasPathology(otherP.context))
                    {
                        mergeActions << "Adding new pathology for " + patientIdentifierString;
                        changed |= ChangedPathologyData;
                        continue;
                    }
                    else
                    {
                        Pathology& pa = d.firstPathology(otherP.context);
                        if (pa == otherP)
                        {
                            continue;
                        }
                        mergeActions << "Merging changed pathology for " + patientIdentifierString;
                        qDebug() << "Pathology different:" << p->firstName << p->surname << p->dateOfBirth.toString();

                        changed |= ChangedPathologyData;
                        continue;
                    }
                }
            }
        }
        if (changed == ChangedPatientProperties)
        {
            mergeActions << "Merging patient properties for " + patientIdentifierString;
        }
    }
    qDebug() << "Mergehints/actions" << mergeHints << mergeActions;

    if (mergeActions.isEmpty())
    {
        if (mergeHints.isEmpty())
        {
            QMessageBox::information(0, tr("Keine Änderungen"), tr("Keine Änderungen zum Zusammenführen"));
            return;
        }
        else
        {
            QMessageBoxResize msgBox(QMessageBox::Information, tr("Keine Änderungen"), tr("Keine Änderungen zum Zusammenführen. Bitte beachten Sie die Hinweise."), QMessageBox::Ok);
            msgBox.setDetailedText(mergeHints.join("\n"));
            msgBox.resize(800, 600);
            msgBox.exec();
            return;
        }
    }
    else
    {
        QMessageBoxResize msgBox(QMessageBox::Question, tr("Zusammenführen"), tr("Sollen %1 Änderungen durchgeführt werden?").arg(mergeActions.size()), QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDetailedText(mergeActions.join("\n") + "\n" + mergeHints.join(("\n")));
        msgBox.resize(800, 600);
        if (msgBox.exec() != QMessageBox::Ok)
        {
            return;
        }
    }

    DatabaseAccess taccess;
    DatabaseTransaction transaction(&taccess);
    foreach (const Patient& other, patients)
    {
        Patient::Ptr p;
        ChangeFlags changed = ChangedNothing;
        QList<Patient::Ptr> ps = findPatients(other);
        if (ps.isEmpty())
        {
            qDebug() << "New patient merged" << other.surname;
            Patient newValues(other);
            newValues.id = 0; // will not be added to db if it has an id
            newValues.diseases.clear(); // merge below, afterwards
            p = addPatient(newValues);
            if (!p)
            {
                qDebug() << "Failed to add patient when merging" << other.firstName << other.surname;
                continue;
            }
            changed = ChangedAll;
        }
        else
        {
            p = ps.first();
            if (ps.size() > 1)
            {
                qWarning() << "Multiple patients found for" << p->firstName << p->surname << p->dateOfBirth;
            }
        }

        QString patientIdentifierString = p->firstName + " " + p->surname + ", " + p->dateOfBirth.toString();

        if (p->patientProperties != other.patientProperties)
        {
            p->patientProperties.merge(other.patientProperties);
            changed |= ChangedPatientProperties;
        }
        foreach (const Disease& otherD, other.diseases)
        {
            int diseaseIndex;
            for (diseaseIndex=0; diseaseIndex<p->diseases.size(); ++diseaseIndex)
            {
                if (p->diseases[diseaseIndex].entity() == otherD.entity())
                {
                    break;
                }
            }
            if (diseaseIndex == p->diseases.size())
            {
                if (p->diseases.size())
                {
                    qDebug() << "Second entity NOT copied automatically" << patientIdentifierString;
                }
                else
                {
                    qDebug() << "Copy new disease" << patientIdentifierString;
                    p->diseases << otherD;
                    Disease& d = p->diseases.last();
                    // reset id trans-database
                    d.id = 0;
                    for (int o=0; o<d.pathologies.size(); o++)
                    {
                        d.pathologies[o].id = 0;
                    }
                    changed |= ChangedDiseaseMetadata | ChangedDiseaseProperties | ChangedDiseaseHistory | ChangedPathologyData;
                }
            }
            else
            {
                Disease& d = p->diseases[diseaseIndex];
                // ! initialDiagnosis: Merge if history changed
                // ! initialTNM: Merge if history changed
                // ! diseaseProperties: Handle history
                if (d.diseaseProperties != otherD.diseaseProperties)
                {
                    DiseaseHistory h = d.history;
                    DiseaseHistory otherH = otherD.history;
                    if (checkHistoryShouldBeReplaced(d, otherH, h, patientIdentifierString))
                    {
                        d.history = otherH;
                        if (otherD.initialDiagnosis.isValid())
                        {
                            d.initialDiagnosis = otherD.initialDiagnosis;
                        }
                        if (!otherD.initialTNM.toText().isEmpty())
                            d.initialTNM = otherD.initialTNM;
                    }
                }
                foreach (const Pathology& otherP, otherD.pathologies)
                {
                    if (!d.hasPathology(otherP.context))
                    {
                        qDebug() << "Copy new pathology" << patientIdentifierString;
                        d.pathologies << otherP;
                        // reset id trans-database
                        d.pathologies.last().id = 0;
                        changed |= ChangedPathologyData;
                    }
                    else
                    {
                        Pathology& pa = d.firstPathology(otherP.context);
                        if (pa == otherP)
                        {
                            continue;
                        }
                        qDebug() << "Merging pathology" << patientIdentifierString << pa.context;
                        pa.entity = otherP.entity;
                        pa.sampleOrigin = otherP.sampleOrigin;
                        pa.date = otherP.date;
                        pa.properties.merge(otherP.properties);
                        changed |= ChangedPathologyData;
                    }
                }
            }
        }
        updateData(p, changed);
    }
}
