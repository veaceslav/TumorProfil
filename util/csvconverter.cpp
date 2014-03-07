/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 04.04.2012
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

#include "csvfile.h"

// Qt includes

#include <QDebug>

// Local includes

#include "csvconverter.h"
#include "databasetransaction.h"
#include "disease.h"
#include "patient.h"
#include "patientmanager.h"
#include "pathologypropertyinfo.h"

void CSVConverter::execute()
{
    DatabaseTransaction transaction;
    CSVConverter converter;
    //converter.convertColon("C:\\Users\\wiesweg\\Documents\\CRCpts04042012.csv");
    //converter.convertBGJ389("C:\\Users\\wiesweg\\Documents\\Tumorprofil\\Screening BGJ389.csv");
    //converter.convertBestRx("C:\\Users\\wiesweg\\Documents\\Tumorprofil\\WTZ und F-Klinik BEST RX.csv");
    converter.readTNM("C:\\Users\\wiesweg\\Documents\\Tumorprofil TNM 06032014.csv");
}

CSVConverter::CSVConverter()
{
}

static Property toProperty(PathologyPropertyInfo::Property id, const QVariant& value,
                           const QString& detail = QString())
{
    Property prop;
    PathologyPropertyInfo info(id);
    ValueTypeCategoryInfo catInfo(info.valueType);
    prop.property = info.id;
    prop.value = catInfo.toPropertyValue(value);
    prop.detail = detail;
    return prop;
}

void CSVConverter::convertBestRx(const QString& filename)
{
    CSVFile file;
    if (!file.read(filename))
    {
        return;
    }

    // skip header line
    file.parseNextLine();
    while (!file.atEnd())
    {
        QList<QVariant> data = file.parseNextLine();
        if (data.size() < 40 || data.first().toString().isEmpty())
            continue;
        Patient pat;
        QStringList names = data[0].toString().split(',', QString::SkipEmptyParts);
        if (names.size() != 2)
        {
            qDebug() << "Failed to parse" << data[0];
            continue;
        }
        pat.surname = names.first().trimmed();
        pat.firstName = names.last().trimmed();
        pat.dateOfBirth = data[5].toDate();
        pat.gender = (data[10].toString() == "M") ? Patient::Male : Patient::Female;
        if (!pat.isValid())
        {
            qDebug() << "invalid patient" << pat.surname;
            continue;
        }

        QString entity = data[7].toString();
        Pathology path;

        path.context = PathologyContextInfo(PathologyContextInfo::BestRx).id;
        path.entity  = Pathology::Breast;
        QDate date;
        QList<int> fields;
        fields << 17 << 21 << 25;
        foreach (int f, fields)
        {
            if (data[f].isValid() && data[f].type() == QVariant::Date)
            {
                path.date = data[f].toDate();
                break;
            }
        }
        if (!path.date.isValid())
        {
            qDebug() << "Pathology date not available" << pat.surname;
            //continue;
        }

        QString sampleOrigin = data[35].toString();
        if (sampleOrigin.contains("Primär") || sampleOrigin.contains("Zytologie"))
        {
            path.sampleOrigin = Pathology::Primary;
        }
        else if (sampleOrigin == "LK")
        {
            path.sampleOrigin = Pathology::LocalLymphNode;
        }
        else if (sampleOrigin.contains("Metastase"))
        {
            path.sampleOrigin = Pathology::Metastasis;
        }
        else
        {
            qDebug() << "Unhandled sampleOrigin" << sampleOrigin;
        }

        // FGFR1
        if (data[23].toString().contains("amplified", Qt::CaseInsensitive))
        {
            bool fgfr1Amplified = data[18].toString() == "Amplified";
            path.properties << toProperty(PathologyPropertyInfo::Fish_FGFR1, fgfr1Amplified);
        }

        // PIK3Ca mut.
        if (data[22].toString().contains("Wild Type") || data[22].toString().contains("Mutant"))
        {
            bool pik3mutant = data[22].toString().contains("Mutant");
            QString mutation;
            if (pik3mutant)
            {
                mutation = data[22].toString().remove("Mutant").trimmed();
            }
            path.properties << toProperty(PathologyPropertyInfo::Mut_PIK3CA_10_21, pik3mutant, mutation);
        }

        // PIK3ca Ampl.
        if (data[23].toString().contains("Amplified", Qt::CaseInsensitive))
        {
            bool pik3Amplified = data[23].toString() == "Amplified";
            path.properties << toProperty(PathologyPropertyInfo::Fish_PIK3CA, pik3Amplified);
        }

        // PTEN
        if (data[29].toString().contains("PTEN staining", Qt::CaseInsensitive))
        {
            bool ptenLoss = data[29].toString().contains("PTEN loss");
            path.properties << toProperty(PathologyPropertyInfo::IHC_PTEN, !ptenLoss);
        }

        // ER, PR, Her2
        if (data[36].toString() == "+" || data[36].toString() == "-")
        {
            bool er = data[36].toString() == "+";
            path.properties << toProperty(PathologyPropertyInfo::IHC_ER, er);
        }
        if (data[37].toString() == "+" || data[37].toString() == "-")
        {
            bool pr = data[37].toString() == "+";
            path.properties << toProperty(PathologyPropertyInfo::IHC_PR, pr);
        }
        if (data[38].toString().contains("+") || data[38].toString() == "0")
        {
            int her2ihc = data[38].toString().remove("+").toInt();
            path.properties << toProperty(PathologyPropertyInfo::IHC_HER2_DAKO, her2ihc);
        }
        if (data[39].toString().contains("amplified", Qt::CaseInsensitive))
        {
            bool her2fish = data[39].toString() == "amplified";
            path.properties << toProperty(PathologyPropertyInfo::Fish_HER2, her2fish);
        }

        /*qDebug() << pat.surname << pat.firstName;
        foreach (const Property& prop, path.properties)
        {
            PathologyPropertyInfo info = PathologyPropertyInfo::info(prop.property);
            ValueTypeCategoryInfo catInfo(info);
            qDebug() << info.label << catInfo.toString(catInfo.toValue(prop.value)) << prop.detail;
        }*/

        QList<Patient::Ptr> ps = PatientManager::instance()->findPatients(pat);
        Patient::Ptr p;
        if (ps.isEmpty())
        {
            p = PatientManager::instance()->addPatient(pat);
        }
        else
        {
            p = ps.first();
        }

        if (!p->hasDisease())
        {
            p->diseases << Disease();
        }
        p->diseases.first().pathologies << path;
        if (!p->diseases.first().initialDiagnosis.isValid())
        {
            bool ok;
            int year = data[31].toInt(&ok);
            if (ok)
            {
                p->diseases.first().initialDiagnosis = QDate(year, 1, 1);
            }
        }

        PatientManager::instance()->updateData(p,
                                               PatientManager::ChangedPathologyData |
                                               PatientManager::ChangedDiseaseMetadata |
                                               PatientManager::ChangedPatientProperties);
    }
}

void CSVConverter::convertBGJ389(const QString& filename)
{
    CSVFile file;
    if (!file.read(filename))
    {
        return;
    }

    // skip header line
    file.parseNextLine();
    while (!file.atEnd())
    {
        QList<QVariant> data = file.parseNextLine();
        if (data.size() < 10 || data.first().type() != QVariant::Int)
            continue;
        Patient pat;
        pat.surname = data[2].toString();
        pat.firstName = data[3].toString();
        pat.dateOfBirth = data[5].toDate();
        pat.gender = (data[4].toString() == "m") ? Patient::Male : Patient::Female;
        if (!pat.isValid())
        {
            qDebug() << "invalid patient" << pat.surname;
            continue;
        }

        QString entity = data[7].toString();
        Pathology path;

        path.context = PathologyContextInfo(PathologyContextInfo::ScreeningBGJ398).id;

        if (entity == "NSCLC")
            path.entity = Pathology::PulmonarySquamous;
        else if (entity == "Mamma")
            path.entity = Pathology::Breast;
        else if (entity == "Bladder")
            path.entity = Pathology::TransitionalCell;
        else
        {
            qDebug() << "Unhandled entity" << entity;
            continue;
        }

        QString sampleOrigin = data[10].toString();
        if (sampleOrigin == "Primär")
        {
            path.sampleOrigin = Pathology::Primary;
        }
        else if (sampleOrigin == "LK")
        {
            path.sampleOrigin = Pathology::LocalLymphNode;
        }
        else if (sampleOrigin.contains("Metastase"))
        {
            path.sampleOrigin = Pathology::Metastasis;
        }
        else
        {
            qDebug() << "Unhandled sampleOrigin" << sampleOrigin;
            continue;
        }

        QString result = data[15].toString();

        bool hasAmplification;
        if (result.contains("neg"))
        {
            hasAmplification = false;
        }
        else if (result.contains("pos"))
        {
            hasAmplification = true;
        }
        else // "NA"
        {
            qDebug() << "invalid result" << result;
            continue;
        }

        Property prop;
        PathologyPropertyInfo info(PathologyPropertyInfo::Fish_FGFR1);
        ValueTypeCategoryInfo catInfo(info.valueType);
        prop.property = info.id;
        prop.value = catInfo.toPropertyValue(hasAmplification);
        path.properties << prop;

        //qDebug() << pat.surname << path.entity << path.sampleOrigin << hasAmplification;

        QList<Patient::Ptr> ps = PatientManager::instance()->findPatients(pat);
        Patient::Ptr p;
        if (ps.isEmpty())
        {
            p = PatientManager::instance()->addPatient(pat);
        }
        else
        {
            p = ps.first();
        }

        if (!p->hasDisease())
        {
            p->diseases << Disease();
        }
        p->diseases.first().pathologies << path;

        PatientManager::instance()->updateData(p,
                                               PatientManager::ChangedPathologyData |
                                               PatientManager::ChangedDiseaseMetadata |
                                               PatientManager::ChangedPatientProperties);
    }
}


void CSVConverter::convertColon(const QString& filename)
{
    CSVFile file;
    if (!file.read(filename))
    {
        return;
    }

    // skip header line
    file.parseNextLine();
    while (!file.atEnd())
    {
        QList<QVariant> data = file.parseNextLine();
        if (data.size() < 10 || data.first().toString().isEmpty())
            continue;
        Patient pat;
        pat.surname = data[0].toString();
        pat.firstName = data[1].toString();
        pat.dateOfBirth = data[2].toDate();
        pat.gender = (data[3].toInt() == 1) ? Patient::Male : Patient::Female;
        QList<Patient::Ptr> ps = PatientManager::instance()->findPatients(pat);
        if (!ps.isEmpty())
        {
            continue;
        }
    }
}

void CSVConverter::readTNM(const QString& filename)
{
    CSVFile file;
    if (!file.read(filename))
    {
        return;
    }

    while (!file.atEnd())
    {
        QList<QVariant> data = file.parseNextLine();
        if (data.size() < 4 || data.first().toString().isEmpty())
        {
            qDebug() << "TNM Reader: Invalid line" << data;
            continue;
        }
        Patient pat;
        pat.surname = data[0].toString();
        pat.firstName = data[1].toString();
        pat.dateOfBirth = data[2].toDate();
        pat.gender = Patient::UnknownGender;
        QList<Patient::Ptr> ps = PatientManager::instance()->findPatients(pat);
        if (ps.isEmpty())
        {
            qDebug() << "TNM Reader: Patient not found" << pat.surname << pat.firstName << pat.dateOfBirth;
            continue;
        }
        Patient::Ptr p = ps.first();
        if (p->diseases.isEmpty())
        {
            qDebug() << "TNM Reader: Patient"<< pat.surname << pat.firstName << pat.dateOfBirth << "has no disease";
        }
        if (p->diseases.size() > 1)
        {
            qDebug() << "TNM Reader: Patient"<< pat.surname << pat.firstName << pat.dateOfBirth << "has multiple diseases, taking first";
        }
        Disease& dis = p->firstDisease();
        dis.initialTNM.setTNM(data[3].toString());
        //PatientManager::instance()->updateData(p, PatientManager::ChangedDiseaseMetadata);
    }
}
