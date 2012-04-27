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
#include "disease.h"
#include "patient.h"
#include "patientmanager.h"
#include "pathologypropertyinfo.h"

void CSVConverter::execute()
{
    CSVConverter converter;
    //converter.convertColon("C:\\Users\\wiesweg\\Documents\\CRCpts04042012.csv");
    converter.convertBGJ389("C:\\Users\\wiesweg\\Documents\\Tumorprofil\\Screening BGJ389.csv");
}

CSVConverter::CSVConverter()
{
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

        PatientManager::instance()->updateData(p);
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
