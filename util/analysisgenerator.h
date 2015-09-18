/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 10.03.2014
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

#ifndef ANALYSISGENERATOR_H
#define ANALYSISGENERATOR_H

#include "csvfile.h"
#include "pathologypropertyinfo.h"
#include "patient.h"
#include "history/historyiterator.h"

class Disease;


class AnalysisGenerator : HistoryProofreader
{
public:
    AnalysisGenerator();

    void her2();
    void her2therapy();
    void findPikBrafTherapy();
    void cmetListe();
    void fishRatioListe();
    void crc2015();
    void nsclcSNCNTrialFinalFromCSV();
    void ros1Project();

protected:

    QVariant writePathologyProperty(const Disease& disease, PathologyPropertyInfo::Property id);
    QVariant writeDetailValue(const Disease& disease, PathologyPropertyInfo::Property id);
    bool hasDetailValue(const Disease& disease, PathologyPropertyInfo::Property id);
    QVariant writeMutationAsDetail(const Disease& disease, PathologyPropertyInfo::Property id);
    void writeIHCPropertySplit(const Disease& disease, PathologyPropertyInfo::Property id);
    void writeIHCIsPositive(const Disease& disease, PathologyPropertyInfo::Property id);
    virtual void problem(const HistoryElement* element, const QString& problem);
    QList<Patient::Ptr> patientsFromCSV(const QString& path);

    CSVFile m_file;
    Patient::Ptr m_currentPatient;
};

#endif // ANALYSISGENERATOR_H
