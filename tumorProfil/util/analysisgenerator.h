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
    void nsclcSCNE21ListFromCSV();
    void nsclcSCNE21PathologyDates();
    void nsclcSCNE21ActionableResults();
    void ros1Project();
    void listsForRadiologyProject();
    void cmetListFromCSV();

protected:

    QVariant writePathologyProperty(const Disease& disease, PathologyPropertyInfo::Property id);
    QVariant writeDetailValue(const Disease& disease, PathologyPropertyInfo::Property id);
    bool hasDetailValue(const Disease& disease, PathologyPropertyInfo::Property id);
    QVariant writeMutationAsDetail(const Disease& disease, PathologyPropertyInfo::Property id);
    void writeIHCPropertySplit(const Disease& disease, PathologyPropertyInfo::Property id);
    void writeIHCIsPositive(const Disease& disease, PathologyPropertyInfo::Property id);
    QList<Patient::Ptr> patientsFromCSV(const QString& path);
    void writeActionableCombinations(const QList<Patient::Ptr>& patients);
    void reportTTF(const QList<QDate>& ctxLineDates, const OSIterator& currentStateIterator, int line, const QDate& sharpBegin = QDate());

    // HistoryProofreader
    virtual void problem(const HistoryElement* element, const QString& problem);

    CSVFile m_file;
    Patient::Ptr m_currentPatient;
};

#endif // ANALYSISGENERATOR_H
