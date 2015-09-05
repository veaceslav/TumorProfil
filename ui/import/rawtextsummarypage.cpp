/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 03.08.2015
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

#include "rawtextsummarypage.h"

#include <QAbstractButton>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>

#include "importwizard.h"
#include "pathologyparser.h"
#include "patientparsepage.h"

RawTextSummaryPage::RawTextSummaryPage()
{
    setTitle(tr("Eingabe erfolgt"));
    setSubTitle(tr("Befunde wurden eingegeben und prozessiert"));

    QVBoxLayout* layout = new QVBoxLayout;
    QLabel* label = new QLabel(tr("Es wurden Befunde von folgenden Patienten eingegeben:"));
    layout->addWidget(label);
    listWidget = new QListWidget;
    layout->addWidget(listWidget);
    setLayout(layout);
}

void RawTextSummaryPage::initializePage()
{
    PathologyParser parser;
    QList<PatientParseResults> results = parser.parse(field("rawText").toString());

    foreach (const PatientParseResults& result, results)
    {
        QString patientDisplay = tr("%1 %2 (geb. am %3)").arg(result.patientData.firstName, result.patientData.surname, result.patientData.dateOfBirth.toString(tr("dd.MM.yyyy")));
        listWidget->addItem(patientDisplay);

        PatientParsePage* page = new PatientParsePage(result);
        wizard()->addPage(page);
    }
}

void RawTextSummaryPage::cleanupPage()
{
    listWidget->clear();
    foreach (int id, wizard()->pageIds())
    {
        if (id > wizard()->currentId())
        {
            wizard()->removePage(id);
        }
    }
}
