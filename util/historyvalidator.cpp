/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 02.04.2014
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

#include "historyvalidator.h"

#include <QDebug>

#include "combinedvalue.h"
#include "diseasehistory.h"
#include "history/historyiterator.h"
#include "patientmanager.h"
#include "patientmodel.h"
#include "patientpropertyfiltermodel.h"
#include "patientpropertymodelviewadapter.h"

HistoryValidator::HistoryValidator()
{
}

void HistoryValidator::validatePAC()
{
    PatientPropertyModelViewAdapter models;
    models.setReportType(PatientPropertyModelViewAdapter::PulmonaryAdenoIHCMut);

    const int size = models.filterModel()->rowCount();
    for (int i=0; i<size; i++)
    {
        p = PatientModel::retrievePatient(models.filterModel()->index(i, 0));
        const Disease& disease = p->firstDisease();
        const DiseaseHistory& history = disease.history();

        if (p->surname.contains("Dktk"))
        {
            continue;
        }
        if (history.isEmpty())
        {
            continue;
        }

        OSIterator it(disease);
        it.setProofreader(this);
        if (it.days(OSIterator::FromFirstTherapy) < 0)
        {
            report(p, "Negative OS");
        }
    }
}


void HistoryValidator::report(const Patient::Ptr& p, const QString& problem)
{
    qDebug() << p->firstName << p->surname << p->dateOfBirth.toString("dd.MM.yyyy") << problem;
}

void HistoryValidator::problem(const HistoryElement*, const QString& problem)
{
    return;
    qDebug() << p->firstName << p->surname << p->dateOfBirth.toString("dd.MM.yyyy")  << problem;
}
