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

#include "importwizard.h"

#include <QApplication>

#include "databaseoperationgroup.h"
#include "patientparsepage.h"
#include "rawtextenterpage.h"
#include "rawtextsummarypage.h"

ImportWizard::ImportWizard()
{
    addPage(new RawTextEnterPage);
    addPage(new RawTextSummaryPage);

    connect(this, &ImportWizard::finished, this, &ImportWizard::wizardFinished);
}

void ImportWizard::wizardFinished()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    DatabaseOperationGroup group;
    foreach (int id, pageIds())
    {
        PatientParsePage* parsePage = qobject_cast<PatientParsePage*>(page(id));
        if (parsePage)
        {
            parsePage->saveData();
        }
        group.allowLift();
    }
    QApplication::restoreOverrideCursor();
}

