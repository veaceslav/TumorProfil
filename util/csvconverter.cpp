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
#include "patient.h"
#include "patientmanager.h"

void CSVConverter::execute()
{
    CSVConverter converter;
    converter.convertColon("C:\\Users\\wiesweg\\Documents\\CRCpts04042012.csv");
}

CSVConverter::CSVConverter()
{
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
