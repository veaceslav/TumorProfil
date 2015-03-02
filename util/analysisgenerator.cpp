/* ============================================================
 *
 * This m_file is a part of Tumorprofil
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

#include "analysisgenerator.h"

#include <QDebug>

#include "combinedvalue.h"
#include "diseasehistory.h"
#include "history/historyiterator.h"
#include "ihcscore.h"
#include "patientmanager.h"
#include "patientmodel.h"
#include "patientpropertyfiltermodel.h"
#include "patientpropertymodelviewadapter.h"

AnalysisGenerator::AnalysisGenerator()
{
}

QVariant AnalysisGenerator::writePathologyProperty(const Disease& disease, PathologyPropertyInfo::Property id)
{
    PathologyPropertyInfo info(id);
    Property prop = disease.pathologyProperty(info.id);
    if (prop.isNull())
    {
        m_file << QString();
        return QVariant();
    }
    ValueTypeCategoryInfo typeInfo(info.valueType);
    QVariant value = typeInfo.toVariantData(prop);
    m_file << value;
    return value;
}

bool AnalysisGenerator::hasDetailValue(const Disease& disease, PathologyPropertyInfo::Property id)
{
    PathologyPropertyInfo info(id);
    Property prop = disease.pathologyProperty(info.id);
    if (prop.isNull() || prop.detail.isEmpty())
    {
        return false;
    }
    return true;
}

QVariant AnalysisGenerator::writeDetailValue(const Disease& disease, PathologyPropertyInfo::Property id)
{
    PathologyPropertyInfo info(id);
    Property prop = disease.pathologyProperty(info.id);
    if (prop.isNull() || prop.detail.isEmpty())
    {
        m_file << QString();
        return QVariant();
    }
    m_file << prop.detail;
    return prop.detail;
}

void AnalysisGenerator::writeIHCPropertySplit(const Disease& disease, PathologyPropertyInfo::Property id)
{
    PathologyPropertyInfo info(id);
    Property prop = disease.pathologyProperty(info.id);
    ValueTypeCategoryInfo typeInfo(info.valueType);
    IHCScore score = typeInfo.toIHCScore(prop);
    if (prop.isNull() || !score.isValid())
    {
        m_file << QVariant();
        m_file << QVariant();
        return;
    }
    m_file << score.colorIntensity;
    m_file << score.positiveRatio();
}

void AnalysisGenerator::writeIHCIsPositive(const Disease& disease, PathologyPropertyInfo::Property id)
{
    PathologyPropertyInfo info(id);
    Property prop = disease.pathologyProperty(info.id);
    ValueTypeCategoryInfo typeInfo(info.valueType);
    IHCScore score = typeInfo.toIHCScore(prop);
    if (prop.isNull() || !score.isValid())
    {
        m_file << QVariant();
        return;
    }
    m_file << score.isPositive(info.property);
}

void AnalysisGenerator::her2()
{
    PatientPropertyModelViewAdapter models;
    models.setReportType(PatientPropertyModelViewAdapter::PulmonaryAdenoIHCMut);

    //m_file.openForWriting("C:\\Users\\wiesweg\\Documents\\Tumorprofil\\HER2-Auswertung 03042014.csv");
    m_file.openForWriting("/home/marcel/Dokumente/Tumorprofil/HER2-Auswertung 01092014.csv");

    const int reportedLines = 5;

    // Header
    m_file << "Nachname"; // 1
    m_file << "Vorname"; // 2
    m_file << "Geburtsdatum"; // 3
    m_file << "M"; // 4
    m_file << "HER2 pos."; // 5
    m_file << "HER2 DAKO"; // 6
    m_file << "HER2 FISH"; // 7
    m_file << "HER2 TingReis"; // 8
    m_file << "HER2 0vs1vs23Fish"; // 9
    m_file << "HER2 01vs23Fish"; // 10
    m_file << "EGFR"; // 11
    m_file << "KRAS"; // 12
    m_file << "PIK3CA"; // 13
    m_file << "BRAF"; // 14
    m_file << "MET overexp"; // 15
    m_file << "MET Hscore"; // 16
    m_file << "pERK"; // 17
    m_file << "pAKT"; // 18
    m_file << "OS"; // 19
    m_file << "OSerreicht"; // 20
    m_file << "Anz Therapielinien"; // 21
    for (int i=0; i<reportedLines; i++) // 22-31
    {
        m_file << QString("TTF") + QString::number(i+1);
        m_file << QString("TTF") + QString::number(i+1) + QString("erreicht");
    }
    m_file << "PatientenID"; // 32
    m_file << "pERK_intens"; // 33
    m_file << "pERK_zahl"; // 34
    m_file << "pAKT_intens";  // 35
    m_file << "pAKT_zahl"; // 36
    m_file << "PTEN"; // 37
    m_file << "PTEN_pos"; // 38
    m_file << "T";
    m_file << "N";
    m_file << "R";
    m_file << "G";
    m_file << "AlterBeiDiagnose";
    m_file << "TNMString";
    m_file << "ALK"; // 45
    m_file << "ALK IHC"; //46
    m_file.newLine();

    QMap<QDate, Patient::Ptr> birthdates;

    const int size = models.filterModel()->rowCount();
    for (int i=0; i<size; i++)
    {
        Patient::Ptr p = PatientModel::retrievePatient(models.filterModel()->index(i, 0));
        m_currentPatient = p;
        const Disease& disease = p->firstDisease();
        const DiseaseHistory& history = disease.history();

        // FOR HER2: Require history
        if (history.isEmpty())
        {
            if (!p->surname.contains("Dktk"))
            {
                //qDebug() << "Empty history for" << p->surname << p->firstName << "skipping for analysis";
            }
            continue;
        }

        /// Metadata
        m_file << p->surname; // 1
        m_file << p->firstName; // 2
        m_file << p->dateOfBirth; // 3

        if (birthdates.contains(p->dateOfBirth))
        {
            qDebug() << "BIRTHDATES NOT UNIQUE" << p->firstName << p->surname << birthdates.value(p->dateOfBirth)->firstName << birthdates.value(p->dateOfBirth)->surname << p->dateOfBirth;
        }
        birthdates.insert(p->dateOfBirth, p);

        /// initial M status
        TNM::MStatus m = disease.initialTNM.mstatus();
        if (m == TNM::Mx)
        {
            qDebug() << "Mx status for" << p->surname << p->firstName << disease.initialTNM.toText();
        }
        if (disease.initialTNM.toText().contains("Mx", Qt::CaseInsensitive))
        {
            qDebug() << "Real Mx status for" << p->surname << p->firstName << disease.initialTNM.toText();
        }
        m_file << (m == TNM::Mx ? QVariant() : QVariant(int(m))); // 4

        /// HER2 status
        CombinedValue her2comb(PathologyPropertyInfo::Comb_HER2);
        her2comb.combine(disease);
        m_file << her2comb.toValue(); // 5

        QVariant her2Dako = writePathologyProperty(disease, PathologyPropertyInfo::IHC_HER2_DAKO); // 6
        // Reuse the code in combinedvalue
        m_file << her2comb.fishResult(disease); // 7
        writePathologyProperty(disease, PathologyPropertyInfo::IHC_HER2); // 8
        // 0vs1vs23Fish
        int zerovs1vs23Fish = her2Dako.toInt();
        if (zerovs1vs23Fish == 3 || her2comb.toValue().toBool())
        {
            zerovs1vs23Fish = 2;
        }
        m_file << zerovs1vs23Fish;
        // 01vs23Fish
        int zero1vs23Fish = 0;
        if (her2Dako.toInt() >= 2 ||  her2comb.toValue().toBool())
        {
            zero1vs23Fish = 1;
        }
        m_file << zero1vs23Fish;

        writePathologyProperty(disease, PathologyPropertyInfo::Mut_EGFR_19_21); // 11
        writePathologyProperty(disease, PathologyPropertyInfo::Mut_KRAS_2); // 12
        writePathologyProperty(disease, PathologyPropertyInfo::Mut_PIK3CA_10_21); // 13
        writePathologyProperty(disease, PathologyPropertyInfo::Mut_BRAF_15); // 14
        CombinedValue metComb(PathologyPropertyInfo::Comb_cMetActivation);
        metComb.combine(disease);
        m_file << metComb.toValue(); // 15
        writePathologyProperty(disease, PathologyPropertyInfo::IHC_cMET); // 16
        writePathologyProperty(disease, PathologyPropertyInfo::IHC_pERK); // 17
        writePathologyProperty(disease, PathologyPropertyInfo::IHC_pAKT); // 18

        /// OS
        OSIterator it(disease);
        it.setProofreader(this);
        m_file << it.days(OSIterator::FromFirstTherapy); // 19
        m_file << (int)it.endpointReached(); // 20

        NewTreatmentLineIterator treatmentLinesIterator;
        treatmentLinesIterator.set(history);
        treatmentLinesIterator.iterateToEnd();
        QDate lastEndDate = history.begin();
        QList<QDate> lineDates, ctxLineDates;
        foreach (const TherapyGroup& group, treatmentLinesIterator.therapies())
        {
            if (group.hasChemotherapy())
            {
                ctxLineDates << group.beginDate();
            }

            // skip groups fully contained in another group
            if (group.effectiveEndDate() > lastEndDate || lastEndDate == history.begin())
            {
                lineDates << group.beginDate();
            }
            lastEndDate = group.effectiveEndDate();
        }
        m_file << ctxLineDates.size(); // 21
        CurrentStateIterator currentStateIterator(history);
        currentStateIterator.setProofreader(this);
        int line = 0;
        for (; line<qMin(reportedLines, ctxLineDates.size()); line++) // 22-31
        {
            QDate begin = ctxLineDates[line];
            QDate end;
            int reachedEndpoint = 0;
            if (ctxLineDates.size() > line+1)
            {
                end = ctxLineDates[line+1];
                reachedEndpoint = 1;
            }
            else
            {
                end = currentStateIterator.effectiveHistoryEnd();
                if (currentStateIterator.effectiveState() == DiseaseState::Deceased)
                {
                    reachedEndpoint = 1;
                }
                else
                {
                    reachedEndpoint = 0;
                }
            }
            m_file << begin.daysTo(end);
            m_file << reachedEndpoint;
        }
        for (; line < reportedLines; line++)
        {
            m_file << QVariant();
            m_file << 0;
        }

        m_file << p->id; // 32
        writeIHCPropertySplit(disease, PathologyPropertyInfo::IHC_pERK); // 33-34
        writeIHCPropertySplit(disease, PathologyPropertyInfo::IHC_pAKT); // 35-36
        writePathologyProperty(disease, PathologyPropertyInfo::IHC_PTEN); // 37
        writeIHCIsPositive(disease, PathologyPropertyInfo::IHC_PTEN); // 38

        m_file << disease.initialTNM.Tnumber();
        m_file << disease.initialTNM.Nnumber();
        m_file << (disease.initialTNM.m_pTNM.R == 'x' ? QVariant() : QVariant(QString(disease.initialTNM.m_pTNM.R)));
        m_file << (disease.initialTNM.m_pTNM.G == 'x' ? QVariant() : QVariant(QString(disease.initialTNM.m_pTNM.G)));

        m_file << (p->dateOfBirth.daysTo(disease.initialDiagnosis) / 365.0);

        m_file << disease.initialTNM.toText();

        writePathologyProperty(disease, PathologyPropertyInfo::Fish_ALK); // 45
        writePathologyProperty(disease, PathologyPropertyInfo::IHC_ALK); // 46

        m_file.newLine();
    }
    m_currentPatient = Patient::Ptr();

    m_file.finishWriting();
}

QList<Patient::Ptr> AnalysisGenerator::patientsFromCSV(const QString &path)
{
    QList<Patient::Ptr> patients;
    CSVFile source;
    if (!source.read(path))
    {
        qDebug() << "Failed to open source file";
        return patients;
    }
    // skip header line
    source.parseNextLine();
    while (!source.atEnd())
    {
        QList<QVariant> data = source.parseNextLine();
        if (data.size() < 4)
            continue;
        QDate dob = QDate::fromString(data[3].toString(), "yyyy-MM-dd");
        if (!dob.isValid())
        {
            dob = QDate::fromString(data[3].toString(), "M/d/yyyy");
            if (!dob.isValid())
            {
                qDebug() << "Failed to parse d o b" << data[3].toString();
                continue;
            }
        }
        QString surname = data[1].toString();
        // special cases in HER2 file
        if (surname == "Muenchow") surname = "Münchow";
        if (surname == "Huettmann") surname = "Hüttmann";
        QList<Patient::Ptr> candidates = PatientManager::instance()->findPatients(surname, data[2].toString(), dob);
        if (candidates.isEmpty())
        {
            qDebug() << "Did not identify" << data[1];
            continue;
        }
        else if (candidates.size() > 1)
        {
            qDebug() << "Multiple pts for" << data[1];
        }
        if (candidates.first()->id != data[0].toInt())
        {
            qDebug() << "Id mismatch" << data[1].toString() << data[2].toString() << data[0].toInt() << candidates.first()->id;
        }
        patients << candidates.first();
    }
    return patients;
}

void AnalysisGenerator::findPikBrafTherapy()
{
    QList<Patient::Ptr> patients = patientsFromCSV("/home/marcel/Dokumente/Tumorprofil/Her2-Projekt/Her2_NSCLC_Enddatei_III_IV.csv");

    QSet<QString> therapies;
    therapies << "BKM120" << "Dabrafenib" << "Vemurafenib";
    foreach (Patient::Ptr p, patients)
    {
        const Disease& disease = p->firstDisease();
        const DiseaseHistory& history = disease.history();
        NewTreatmentLineIterator treatmentLinesIterator;
        treatmentLinesIterator.setProofreader(this);
        treatmentLinesIterator.set(history);
        treatmentLinesIterator.iterateToEnd();

        foreach (const TherapyGroup& group, treatmentLinesIterator.therapies())
        {
            if (!group.substances().intersect(therapies).isEmpty())
            {
                qDebug() << p->firstName << p->surname << group.substances();
                break;
            }
        }
    }
}

void AnalysisGenerator::her2therapy()
{
    QList<Patient::Ptr> patients = patientsFromCSV("/home/marcel/Dokumente/Tumorprofil/Her2-Projekt/Her2_NSCLC_Enddatei_III_IV.csv");
    m_file.openForWriting("/home/marcel/Dokumente/Tumorprofil/HER2 Therapiedaten 19122014.csv");
    m_file << "id";
    m_file << "Nachname";
    m_file << "Vorname";
    m_file << "Geburtsdatum";
    m_file << "PemFirstLine";
    m_file << "EGFR_TKI";
    m_file.newLine();

    qDebug() << "Identified" << patients.size();
    QSet<QString> overallSubstances;
    int numberPemFirstLine = 0;

    foreach (Patient::Ptr p, patients)
    {
        m_file << p->id;
        m_file << p->surname;
        m_file << p->firstName;
        m_file << p->dateOfBirth;

        const Disease& disease = p->firstDisease();
        const DiseaseHistory& history = disease.history();
        TNM::MStatus m = disease.initialTNM.mstatus();

        QDate firstProgress;
        if (m == TNM::M0)
        {
            ProgressionIterator pit;
            pit.set(history);
            bool anyProgress = false;
            if (pit.next() == HistoryIterator::Match)
            {
                firstProgress = pit.currentElement()->date;
                anyProgress = true;
            }
            Q_UNUSED(anyProgress)

            /*if (!anyProgress)
            {
                qDebug() << "Never saw any progress with" << p->firstName << p->surname;
            }*/
            /*EffectiveStateIterator esit(history);
            bool sawFollowUp;
            while (esit.next() != HistoryIterator::AtEnd)
            {

            }*/
        }

        NewTreatmentLineIterator treatmentLinesIterator;
        treatmentLinesIterator.setProofreader(this);
        treatmentLinesIterator.set(history);
        treatmentLinesIterator.iterateToEnd();

        QSet<QString> allSubstances;
        foreach (const TherapyGroup& group, treatmentLinesIterator.therapies())
        {
            allSubstances += group.substances();
        }
        overallSubstances += allSubstances;

        TherapyGroup firstLineTherapy, firstLineCTx;
        foreach (const TherapyGroup& group, treatmentLinesIterator.therapies())
        {
            if (firstProgress.isValid() && group.beginDate() < firstProgress)
            {
                continue;
            }

            if (group.hasChemotherapy())
            {
                firstLineCTx = group;
                break;
            }
            // If there is no CTx but a first line therapy (RTx), catch it
            if (firstLineTherapy.isEmpty())
            {
                firstLineTherapy = group;
            }
        }

        /*
        if (firstLineTherapy.isEmpty() && firstLineCTx.isEmpty() && !firstProgress.isValid())
        {
            qDebug() << "No therapy for" << p->firstName << p->surname;
        }
        else if (firstLineCTx.isEmpty() && !firstProgress.isValid())
        {
            qDebug() << "No first line for" << p->firstName << p->surname;
        }
        */

        // Pem 1st line
        if (firstLineCTx.size())
        {
            bool pemFirstLine = firstLineCTx.substances().contains("Pemetrexed");
            m_file << pemFirstLine;
            if (pemFirstLine)
                numberPemFirstLine++;
        }
        else
        {
            m_file << QVariant();
        }

        // any EGFR TKI
        QSet<QString> egfrTKIs;
        egfrTKIs << "AZD9291" << "Afatinib" << "BIBW 2992" << "Erlotinib" << "Gefitinib";
        if (firstLineCTx.size())
        {
            m_file << !allSubstances.intersect(egfrTKIs).isEmpty();
        }
        else
        {
            m_file << QVariant();
        }



        m_file.newLine();
    }
    QList<QString> overallSubstancesList = overallSubstances.toList();
    qSort(overallSubstancesList);
    qDebug() << overallSubstancesList;

    m_file.finishWriting();
}

void AnalysisGenerator::cmetListe()
{
    PatientPropertyModelViewAdapter models;
    models.setReportType(PatientPropertyModelViewAdapter::PulmonaryAdenoIHCMut);

    m_file.openForWriting("/home/marcel/Dokumente/Tumorprofil/cMET Adeno-Ca Stand 24.01.2015.csv");

    // Header
    m_file << "Nachname"; // 1
    m_file << "Vorname"; // 2
    m_file << "Geburtsdatum"; // 3
    m_file << "Befunddatum"; // 4
    m_file << "Erstdiagnose";
    m_file << "MET Hscore";
    m_file << "MET IHC 1+";
    m_file << "MET IHC 2+";
    m_file << "MET IHC 3+";
    m_file.newLine();

    QMap<QDate, Patient::Ptr> birthdates;

    const int size = models.filterModel()->rowCount();
    for (int i=0; i<size; i++)
    {
        Patient::Ptr p = PatientModel::retrievePatient(models.filterModel()->index(i, 0));
        m_currentPatient = p;
        const Disease& disease = p->firstDisease();
        if (!disease.hasProfilePathology())
        {
            continue;
        }
        if (p->surname.startsWith("Dktk", Qt::CaseInsensitive) && p->firstName.trimmed().isEmpty())
        {
            continue;
        }
        PathologyPropertyInfo infoMet(PathologyPropertyInfo::IHC_cMET);
        ValueTypeCategoryInfo ihcTypeMet(PathologyPropertyInfo::IHC_cMET);
        Property prop = disease.pathologyProperty(infoMet.id);
        if (prop.isNull())
        {
            continue;
        }

        /// Metadata
        m_file << p->surname; // 1
        m_file << p->firstName; // 2
        m_file << p->dateOfBirth; // 3

        m_file << disease.firstProfilePathology().date;
        m_file << disease.initialDiagnosis;

        /*
        CombinedValue metComb(PathologyPropertyInfo::Comb_cMetIHC3plusScore);
        metComb.combine(disease);
        m_file << metComb.toValue();
        */

        writePathologyProperty(disease, PathologyPropertyInfo::IHC_cMET); // 16
        HScore hscore = ihcTypeMet.toMedicalValue(prop).value<HScore>();
        m_file << hscore.percentageWeak();
        m_file << hscore.percentageMedium();
        m_file << hscore.percentageStrong();

        m_file.newLine();
    }
    m_file.finishWriting();
}

void AnalysisGenerator::fishRatioListe()
{
    PatientPropertyModelViewAdapter models;
    models.setReportType(PatientPropertyModelViewAdapter::PulmonarySquamousIHCMut);

    m_file.openForWriting("/home/marcel/Dokumente/Tumorprofil/FISH PEC Ratio Stand 12.02.2015.csv");

    // Header
    m_file << "Nachname"; // 1
    m_file << "Vorname"; // 2
    m_file << "Geburtsdatum"; // 3
    m_file << "Befunddatum"; // 4
    m_file << "HER2 ratio";
    m_file << "PIK3CA ratio";
    m_file << "FGFR ratio";
    m_file << "Alk ratio";
    m_file << "Ros1 ratio";
    m_file.newLine();

    QMap<QDate, Patient::Ptr> birthdates;

    const int size = models.filterModel()->rowCount();
    for (int i=0; i<size; i++)
    {
        Patient::Ptr p = PatientModel::retrievePatient(models.filterModel()->index(i, 0));
        m_currentPatient = p;
        const Disease& disease = p->firstDisease();
        if (!disease.hasProfilePathology())
        {
            continue;
        }
        if (p->surname.startsWith("Dktk", Qt::CaseInsensitive) && p->firstName.trimmed().isEmpty())
        {
            continue;
        }
        if (!hasDetailValue(disease, PathologyPropertyInfo::Fish_HER2)
                && !hasDetailValue(disease, PathologyPropertyInfo::Fish_PIK3CA)
                && !hasDetailValue(disease, PathologyPropertyInfo::Fish_FGFR1)
                && !hasDetailValue(disease, PathologyPropertyInfo::Fish_ALK)
                && !hasDetailValue(disease, PathologyPropertyInfo::Fish_ROS1))
        {
            continue;
        }

        /// Metadata
        m_file << p->surname; // 1
        m_file << p->firstName; // 2
        m_file << p->dateOfBirth; // 3

        m_file << disease.firstProfilePathology().date;

        /*
        CombinedValue metComb(PathologyPropertyInfo::Comb_cMetIHC3plusScore);
        metComb.combine(disease);
        m_file << metComb.toValue();
        */

        writeDetailValue(disease, PathologyPropertyInfo::Fish_HER2);
        writeDetailValue(disease, PathologyPropertyInfo::Fish_PIK3CA);
        writeDetailValue(disease, PathologyPropertyInfo::Fish_FGFR1);
        writeDetailValue(disease, PathologyPropertyInfo::Fish_ALK);
        writeDetailValue(disease, PathologyPropertyInfo::Fish_ROS1);

        m_file.newLine();
    }
    m_file.finishWriting();
}

void AnalysisGenerator::problem(const HistoryElement *, const QString &problem)
{
    return; // be silent
    if (!m_currentPatient)
    {
        qDebug() << problem;
    }
    qDebug() << m_currentPatient->firstName + " " + m_currentPatient->surname + ": " << problem;
}
