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
#include <QVector>

#include "actionableresultchecker.h"
#include "combinedvalue.h"
#include "dataaggregator.h"
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

QVariant AnalysisGenerator::writeMutationAsDetail(const Disease& disease, PathologyPropertyInfo::Property id)
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
    if (value.toBool())
    {
        if (prop.detail.isEmpty())
        {
            m_file << "Mutation";
        }
        else
        {
            m_file << prop.detail;
        }
    }
    else
    {
        m_file << "WT";
    }
    return value;
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
        const DiseaseHistory& history = disease.history;

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
    // Wants a header line and a first column containing (optionally) the id
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
        QDate dob;
        if (data[3].type() == QVariant::Date)
        {
            dob = data[3].toDate();
        }
        else
        {
            QString dateString = data[3].toString();
            if (dateString.contains('-'))
            {
                dob = QDate::fromString(data[3].toString(), "yyyy-MM-dd");
            }
            else if (dateString.contains('/'))
            {
                dob = QDate::fromString(data[3].toString(), "M/d/yyyy");
            }
        }
        if (!dob.isValid())
        {
            qDebug() << "Failed to parse d o b" << data[3] << data[1].toString();
            continue;
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
        if (!data[0].toString().isEmpty() && candidates.first()->id != data[0].toInt())
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
        const DiseaseHistory& history = disease.history;
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
        const DiseaseHistory& history = disease.history;
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

    m_file.openForWriting("/home/marcel/Dokumente/Tumorprofil/cMET Adeno-Ca Stand 04.04.2015.csv");

    // Header
    m_file << "Nachname"; // 1
    m_file << "Vorname"; // 2
    m_file << "Geburtsdatum"; // 3
    m_file << "Befunddatum"; // 4
    m_file << "Erstdiagnose";
    m_file << "Studienscore";
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

        /*if (disease.firstProfilePathology().date < QDate(2014,12,1))
        {
            continue;
        }*/

        /// Metadata
        m_file << p->surname; // 1
        m_file << p->firstName; // 2
        m_file << p->dateOfBirth; // 3

        m_file << disease.firstProfilePathology().date;
        m_file << disease.initialDiagnosis;


        CombinedValue metComb(PathologyPropertyInfo::Comb_cMetIHC3plusScore);
        metComb.combine(disease);
        m_file << metComb.toValue();


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

void AnalysisGenerator::reportTTF(const QList<QDate> &ctxLineDates, const OSIterator& osIterator, int line, const QDate& sharpBegin)
{
    QDate begin;
    if (sharpBegin.isValid())
    {
        // usually, the day of the administration of the substance
        begin = sharpBegin;
    }
    else
    {
        // usually, the beginning of the therapy group
        begin = ctxLineDates[line];
    }
    QDate end;
    int reachedEndpoint = 0;
    // If there is a following line of treatment, TTF is reached per definition
    if (ctxLineDates.size() > line+1)
    {
        end = ctxLineDates[line+1];
        reachedEndpoint = 1;
    }
    else
    {
        end = osIterator.endDate();
        reachedEndpoint = osIterator.endpointReached() ? 1 : 0;
    }
    m_file << begin.daysTo(end);
    m_file << reachedEndpoint;
}

void AnalysisGenerator::crc2015()
{
    PatientPropertyModelViewAdapter models;
    models.setReportType(PatientPropertyModelViewAdapter::CRCIHCMut);

    /** Overall substances
             "Aflibercept"
             "Raltitrexed"
             "INKL-S TRIN-2755-I-001"
             "PIK3-Inhibitor BYL719"
             "BRAF-Inhibitor LGX818"
             ""
             "Tegafur/uracil"
             "Bevacizumab"
             "Folins\u00E4ure"
             "Etoposid"
             "Oxaliplatin"
             "Stimuvax"
             "Simtuzumab"
             "LGX818 (BRAF-Inhibitor) "
             "BYL719 (PI3K-Inhibitor)"
             "Tegafur"
             "Capecitabin"
             "Vemurafenib"
             "Tegafur "
             "5-FU"
             "MEK-Inhibitor"
             "Mitomycin C"
             "Pan-PI3K-Inhibitor BKM120"
             "Hedgehog-Inhibitor LDE 225"
             "Panitumumab"
             "Regorafenib"
             "Mitomycin"
             "Panitunumab"
             "Raltirexed"
             "Irinotecan"
             "Carboplatin"
             "Cetuximab"
             "Cyclophosphamid"
             "BRAF-Inhibitor LGX818 "
             "PI3K-Inhibitor BYL719 "
             " Raltitrexed"
             "Gemcitabin"
    */
    //m_file.openForWriting("C:\\Users\\wiesweg\\Documents\\Tumorprofil\\HER2-Auswertung 03042014.csv");
    m_file.openForWriting("/home/marcel/Dokumente/Tumorprofil/CRC Projekt/Auswertung.csv");

    const int reportedLines = 5;

    // Header
    m_file << "Nachname"; // 1
    m_file << "Vorname"; // 2
    m_file << "Geburtsdatum"; // 3
    m_file << "Geschlecht";
    m_file << "PatientenID";
    m_file << "T";
    m_file << "N";
    m_file << "M";
    m_file << "R";
    m_file << "G";
    m_file << "AlterBeiDiagnose";
    m_file << "TNMString";
    m_file << "pERK";
    m_file << "pERK_intens";
    m_file << "pERK_zahl";
    m_file << "pERK_pos";
    m_file << "pAKT";
    m_file << "pAKT_intens";
    m_file << "pAKT_zahl";
    m_file << "pAKT_pos";
    m_file << "p70S6K";
    m_file << "p70S6K_intens";
    m_file << "p70S6K_zahl";
    m_file << "p70S6K_pos";
    m_file << "PTEN";
    m_file << "PTEN_pos";
    m_file << "MET Hscore";
    m_file << "MET overexp";

    m_file << "RAS";
    m_file << "KRAS";
    m_file << "NRAS";
    m_file << "PIK3CA";
    m_file << "BRAF";

    m_file << "OS";
    m_file << "OSerreicht";
    m_file << "Anz_Therapielinien";
    for (int i=0; i<reportedLines; i++)
    {
        m_file << QString("TTF") + QString::number(i+1);
        m_file << QString("TTF") + QString::number(i+1) + QString("erreicht");
    }

    enum SpecificTherapy
    {
        Oxaliplatin,
        Irinotecan,
        EGFRAntibody,
        AntiangiogeneticTherapy, // adjust Last...

        FirstSpecificTherapy = Oxaliplatin,
        LastSpecificTherapy  = AntiangiogeneticTherapy
    };

    QMultiMap<SpecificTherapy, QString> specificTherapySubstances;
    specificTherapySubstances.insert(Oxaliplatin, "Oxaliplatin");
    specificTherapySubstances.insert(Irinotecan, "Irinotecan");
    specificTherapySubstances.insert(EGFRAntibody, "Cetuximab");
    specificTherapySubstances.insert(EGFRAntibody, "Panitumumab");
    specificTherapySubstances.insert(EGFRAntibody, "Panitunumab");
    specificTherapySubstances.insert(EGFRAntibody, "Vectibix");
    specificTherapySubstances.insert(EGFRAntibody, "Erbitux");
    specificTherapySubstances.insert(AntiangiogeneticTherapy, "Bevacizumab");
    specificTherapySubstances.insert(AntiangiogeneticTherapy, "Avastin");
    specificTherapySubstances.insert(AntiangiogeneticTherapy, "Aflibercept");
    specificTherapySubstances.insert(AntiangiogeneticTherapy, "Zaltrap");
    specificTherapySubstances.insert(AntiangiogeneticTherapy, "Ramucirumab");
    specificTherapySubstances.insert(AntiangiogeneticTherapy, "Cyramza");

    QMap<SpecificTherapy, QString> specificTherapyShortcuts;
    specificTherapyShortcuts.insert(Oxaliplatin, "Ox");
    specificTherapyShortcuts.insert(Irinotecan, "Iri");
    specificTherapyShortcuts.insert(EGFRAntibody, "EGFR_AB");
    specificTherapyShortcuts.insert(EGFRAntibody, "Antiangio");

    for (int i=FirstSpecificTherapy; i<=LastSpecificTherapy; ++i)
    {
        SpecificTherapy specificTherapy = SpecificTherapy(i);
        const QString& shortCut = specificTherapyShortcuts.value(specificTherapy);
        m_file << shortCut + "_line_number";
        m_file << "OS_" + shortCut;
        m_file << "TTF_" + shortCut;
        m_file << "TTF_" + shortCut + "_erreicht";
    }

    m_file << "Therapie_Kategorie";
    m_file << "OS_nach_Rezidiv_oder_initial_palliativer_Situation"; // "endpoint reached" is not different

    m_file.newLine();

    const int size = models.filterModel()->rowCount();
    for (int i=0; i<size; i++)
    {
        Patient::Ptr p = PatientModel::retrievePatient(models.filterModel()->index(i, 0));
        m_currentPatient = p;
        const Disease& disease = p->firstDisease();
        const DiseaseHistory& history = disease.history;

        // Require history
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
        m_file << p->gender;
        m_file << p->id;

        // T, N
        m_file << disease.initialTNM.Tnumber();
        m_file << disease.initialTNM.Nnumber();

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

        // R, G
        m_file << (disease.initialTNM.m_pTNM.R == 'x' ? QVariant() : QVariant(QString(disease.initialTNM.m_pTNM.R)));
        m_file << (disease.initialTNM.m_pTNM.G == 'x' ? QVariant() : QVariant(QString(disease.initialTNM.m_pTNM.G)));

        // Alter bei Diagnose
        m_file << (p->dateOfBirth.daysTo(disease.initialDiagnosis) / 365.0);
        // TNM String
        m_file << disease.initialTNM.toText();

        writePathologyProperty(disease, PathologyPropertyInfo::IHC_pERK);
        writeIHCPropertySplit(disease, PathologyPropertyInfo::IHC_pERK); // two lines
        writeIHCIsPositive(disease, PathologyPropertyInfo::IHC_pERK);

        writePathologyProperty(disease, PathologyPropertyInfo::IHC_pAKT);
        writeIHCPropertySplit(disease, PathologyPropertyInfo::IHC_pAKT); // two lines
        writeIHCIsPositive(disease, PathologyPropertyInfo::IHC_pAKT);

        writePathologyProperty(disease, PathologyPropertyInfo::IHC_pP70S6K);
        writeIHCPropertySplit(disease, PathologyPropertyInfo::IHC_pP70S6K); // two lines
        writeIHCIsPositive(disease, PathologyPropertyInfo::IHC_pP70S6K);

        writePathologyProperty(disease, PathologyPropertyInfo::IHC_PTEN);
        writeIHCIsPositive(disease, PathologyPropertyInfo::IHC_PTEN);

        writePathologyProperty(disease, PathologyPropertyInfo::IHC_cMET);
        CombinedValue metComb(PathologyPropertyInfo::Comb_cMetActivation);
        metComb.combine(disease);
        m_file << metComb.toValue();

        CombinedValue rasComb(PathologyPropertyInfo::Comb_RASMutation);
        rasComb.setMissingValueBehavior(CombinedValue::PragmaticMissingValueBehavior);
        rasComb.combine(disease);
        m_file << rasComb.toValue();
        CombinedValue krasComb(PathologyPropertyInfo::Comb_KRASMutation);
        krasComb.setMissingValueBehavior(CombinedValue::PragmaticMissingValueBehavior);
        krasComb.combine(disease);
        m_file << krasComb.toValue();

        writePathologyProperty(disease, PathologyPropertyInfo::Mut_NRAS_2_4);
        writePathologyProperty(disease, PathologyPropertyInfo::Mut_PIK3CA_10_21);
        writePathologyProperty(disease, PathologyPropertyInfo::Mut_BRAF_15);

        /// OS
        OSIterator osIterator(disease);
        osIterator.setProofreader(this);
        m_file << osIterator.days(OSIterator::FromFirstTherapy);
        m_file << (int)osIterator.endpointReached();

        /// Treatment lines
        NewTreatmentLineIterator treatmentLinesIterator;
        treatmentLinesIterator.setProofreader(this);
        treatmentLinesIterator.set(history);
        treatmentLinesIterator.iterateToEnd();
        QList<TherapyGroup> therapies = treatmentLinesIterator.therapies();

        QDate lastEndDate = history.begin();
        QList<QDate> lineDates, ctxLineDates;
        QVector<HistoryElement*> firstTherapies(LastSpecificTherapy+1);
        QVector<int> firstTherapyLines(LastSpecificTherapy+1);
        foreach (const TherapyGroup& group, therapies)
        {
            if (group.hasChemotherapy())
            {
                ctxLineDates << group.beginDate();
            }

            for (int i=FirstSpecificTherapy; i<=LastSpecificTherapy; ++i)
            {
                SpecificTherapy specificTherapy = SpecificTherapy(i);

                // already found a first line? Continue
                if (firstTherapies[specificTherapy])
                {
                    continue;
                }

                // Try all possible substances, check if this line contains the substance
                foreach (const QString& substance, specificTherapySubstances.values(specificTherapy))
                {
                    if (group.hasSubstance(substance))
                    {
                        foreach (Therapy*t, group)
                        {
                            if (t->elements.hasSubstance(substance))
                            {
                                firstTherapies[specificTherapy] = t;
                                break;
                            }
                        }
                        firstTherapyLines[specificTherapy] = ctxLineDates.size() - 1;
                        break;
                    }
                }
            }

            // skip groups fully contained in another group
            if (group.effectiveEndDate() > lastEndDate || lastEndDate == history.begin())
            {
                lineDates << group.beginDate();
            }
            lastEndDate = group.effectiveEndDate();
        }

        // Number of CTx therapy lines
        m_file << ctxLineDates.size();

        // TTF1-5
        // NOTE: This reports therapy groups. If e.g. an operation is grouped as first-line with a CTx, this takes the first date (usually the surgery) as beginning
        CurrentStateIterator currentStateIterator(history);
        currentStateIterator.setProofreader(this);
        int line = 0;
        for (; line<qMin(reportedLines, ctxLineDates.size()); line++)
        {
            reportTTF(ctxLineDates, osIterator, line);
        }
        // file empty spaces if actual number of lines is less than reported lines
        for (; line < reportedLines; line++)
        {
            m_file << QVariant();
            m_file << QVariant();
        }

        // NOTE: In contrast to TTF1-5, this takes the first administration of the substance as the beginning. In consequence, may differ from the corresponding TTF.
        for (int i=FirstSpecificTherapy; i<=LastSpecificTherapy; ++i)
        {
            SpecificTherapy specificTherapy = SpecificTherapy(i);

            // Did we see a therapy line with that substance?
            if (firstTherapies[specificTherapy])
            {
                // number of first line with this substance
                m_file << firstTherapyLines[specificTherapy] + 1; // index is 0-based, line number is one-based
                // OS
                m_file << osIterator.days(firstTherapies[specificTherapy]);
                // TTF
                reportTTF(ctxLineDates, osIterator, firstTherapyLines[specificTherapy], firstTherapies[specificTherapy]->date);
            }
            else
            {
                m_file << QVariant();
                m_file << QVariant();
                m_file << QVariant();
                m_file << QVariant();
                continue;
            }

        }

        /// Categories grouping
        /// 0 -> no therapy
        /// 1 -> entered follow-up after first-line; no recurrence
        /// 2 -> entered follow-up afger first-line; saw recurrence
        /// 3 -> initially systemic therapy / did never enter follow-up
        int category;
        ProgressionIterator progressionIterator(ProgressionIterator::OnlyRecurrence);
        if (therapies.isEmpty())
        {
             category = 0;
        }
        else
        {
            const TherapyGroup& firstGroup = therapies.first();

            // check if the patient entered follow up after the first-line therapy
            EffectiveStateIterator effectiveStateIterator;
            effectiveStateIterator.set(history, firstGroup.lastTherapy());
            for (; effectiveStateIterator.next() == HistoryIterator::Match; )
            {
                if (effectiveStateIterator.effectiveState() == DiseaseState::FollowUp)
                {
                    break;
                }
            }

            if (effectiveStateIterator.effectiveState() == DiseaseState::FollowUp)
            {
                // Ok, we entered Follow Up at some point.
                // This fulfills the definition of group 1 or 2. All the rest goes into group 3.

                // Now, lets have a look. Do we see recurrence?
                progressionIterator.set(history, effectiveStateIterator.currentElement());
                if (progressionIterator.next() == ProgressionIterator::Match)
                {
                    category = 2;
                }
                else
                {
                    category = 1;
                    // double check
                    for (; effectiveStateIterator.next() == HistoryIterator::Match; )
                    {
                        switch (effectiveStateIterator.effectiveState())
                        {
                        case DiseaseState::Therapy:
                        case DiseaseState::WatchAndWait:
                        case DiseaseState::BestSupportiveCare:
                        {
                            qDebug() << "! No recurrence for" << p->surname << p->firstName << "but state" << effectiveStateIterator.effectiveState() << "after FollowUp. Please check. Assuming recurrent disease.";
                            category = 2;
                            break;
                        }
                        default:
                            break;
                        }
                    }
                }
            }
            else
            {
                category = 3;
            }
        }
        m_file << (category ? QVariant(category) : QVariant());

        // OS after recurrence or initial incurable disease
        switch (category)
        {
        case 0:
        case 1:
            m_file << QVariant();
            break;
        case 2:
            m_file << osIterator.days(progressionIterator.currentElement());
            break;
        case 3:
            m_file << osIterator.days(OSIterator::FromFirstTherapy);
            break;
        }

        m_file.newLine();
    }
    m_currentPatient = Patient::Ptr();

    m_file.finishWriting();
}

void AnalysisGenerator::nsclcSCNE21ListFromCSV()
{
    QList<Patient::Ptr> patients = patientsFromCSV("/home/marcel/Dokumente/Tumorprofil/Novartis SCNE-21/Final/Daten 180 Patienten.csv");
    m_file.openForWriting("/home/marcel/Dokumente/Tumorprofil/Novartis SCNE-21/Final/Patienten Mutationen OS.csv");
    m_file << "id";
    m_file << "Nachname";
    m_file << "Vorname";
    m_file << "Geburtsdatum";
    m_file << "Adeno oder PEC";
    m_file << "EGFR 19 21";
    m_file << "EGFR 18 20";
    m_file << "KRAS";
    m_file << "PIK3CA";
    m_file << "BRAF";
    m_file << "MET overexp";
    m_file << "MET Hscore";
    m_file << "OS";
    m_file << "OSerreicht";
    m_file.newLine();

    qDebug() << "Identified" << patients.size();
    QSet<QString> overallSubstances;

    foreach (Patient::Ptr p, patients)
    {
        m_file << p->id;
        m_file << p->surname;
        m_file << p->firstName;
        m_file << p->dateOfBirth;

        const Disease& disease = p->firstDisease();
        const DiseaseHistory& history = disease.history;

        if (disease.entity() == Pathology::PulmonaryAdeno || disease.entity() == Pathology::PulmonaryAdenosquamous || disease.entity() == Pathology::PulmonaryBronchoalveloar)
        {
            m_file << "Adeno";
        }
        else if (disease.entity() == Pathology::PulmonarySquamous)
        {
            m_file << "PEC";
        }
        else
        {
            m_file << "anderes";
        }

        writeMutationAsDetail(disease, PathologyPropertyInfo::Mut_EGFR_19_21);
        writeMutationAsDetail(disease, PathologyPropertyInfo::Mut_EGFR_18_20);
        writeMutationAsDetail(disease, PathologyPropertyInfo::Mut_KRAS_2);
        writeMutationAsDetail(disease, PathologyPropertyInfo::Mut_PIK3CA_10_21);
        writeMutationAsDetail(disease, PathologyPropertyInfo::Mut_BRAF_15);
        CombinedValue metComb(PathologyPropertyInfo::Comb_cMetActivation);
        metComb.combine(disease);
        m_file << metComb.toValue();
        writePathologyProperty(disease, PathologyPropertyInfo::IHC_cMET);

        if (!history.isEmpty())
        {
            OSIterator it(disease);
            it.setProofreader(this);
            m_file << it.days(OSIterator::FromFirstTherapy);
            m_file << (int)it.endpointReached();
        }
        else
        {
            m_file << QVariant() << QVariant();
        }

        m_file.newLine();
    }

    m_file.finishWriting();
}

void AnalysisGenerator::nsclcSCNE21PathologyDates()
{
    QList<Patient::Ptr> patients = patientsFromCSV("/home/marcel/Dokumente/Tumorprofil/Novartis SCNE-21/Final/Patienten Mutationen OS.csv");
    m_file.openForWriting("/home/marcel/Dokumente/Tumorprofil/Novartis SCNE-21/Final/Patienten Befunddatum.csv");
    m_file << "id";
    m_file << "Nachname";
    m_file << "Vorname";
    m_file << "Geburtsdatum";
    m_file << "Befunddatum";
    m_file.newLine();

    qDebug() << "Identified" << patients.size();

    foreach (Patient::Ptr p, patients)
    {
        m_file << p->id;
        m_file << p->surname;
        m_file << p->firstName;
        m_file << p->dateOfBirth;

        const Disease& disease = p->firstDisease();
        if (!disease.hasProfilePathology())
        {
            m_file << QDate();
        }
        else
        {
            const Pathology& path = disease.firstProfilePathology();
            m_file << path.date;
        }

        m_file.newLine();
    }

    m_file.finishWriting();
}

void AnalysisGenerator::writeActionableCombinations(const QList<Patient::Ptr>& patients)
{
    QMap< QList<PathologyPropertyInfo>, DataAggregator* > actionableCombinations = ActionableResultChecker::actionableCombinations(patients, ActionableResultChecker::IncludeRAS);
    QMap< QList<PathologyPropertyInfo>,  DataAggregator* >::const_iterator it;
    for (it = actionableCombinations.begin(); it != actionableCombinations.end(); ++it)
    {
        QStringList titles;
        foreach (const PathologyPropertyInfo& info, it.key())
        {
            titles << info.plainTextLabel();
        }
        if (titles.isEmpty())
        {
            titles << "Kein relevanter Befund";
        }
        m_file << titles.join(", ");
        m_file << it.value()->values().value(AggregatedDatumInfo(AggregatedDatumInfo::Positive, AggregatedDatumInfo::AbsoluteValue));
        m_file.newLine();
    }
    qDeleteAll(actionableCombinations);
}

void AnalysisGenerator::nsclcSCNE21ActionableResults()
{
    QList<Patient::Ptr> patients = patientsFromCSV("/home/marcel/Dokumente/Tumorprofil/Novartis SCNE-21/Final/Patienten Mutationen OS.csv");
    m_file.openForWriting("/home/marcel/Dokumente/Tumorprofil/Novartis SCNE-21/Final/Actionable results.csv");
    m_file << "Combination";
    m_file << "n";
    m_file.newLine();

    qDebug() << "Identified" << patients.size();
    QList<Patient::Ptr> adc, pec;
    foreach (const Patient::Ptr p, patients)
    {
        if (p->firstDisease().entity() == Pathology::PulmonarySquamous)
        {
            pec << p;
        }
        else
        {
            adc << p;
        }
    }

    m_file << "ADC";
    m_file << adc.size();
    m_file.newLine();
    writeActionableCombinations(adc);
    m_file << "PEC";
    m_file << pec.size();
    m_file.newLine();
    writeActionableCombinations(pec);
    m_file.finishWriting();
}

void AnalysisGenerator::ros1Project()
{
    PatientPropertyModelViewAdapter models;
    models.setReportType(PatientPropertyModelViewAdapter::PulmonaryAdenoIHCMut);

    const int reportedLines = 5;

    QList<Patient::Ptr> ros1patients = patientsFromCSV("/home/marcel/Dokumente/Tumorprofil/ROS1-Projekt/ROS1-Patientenliste.csv");

    m_file.openForWriting("/home/marcel/Dokumente/Tumorprofil/Charlotte Skiba/Datenbankauswertung.csv");
    m_file << "id";
    m_file << "Nachname";
    m_file << "Vorname";
    m_file << "Geburtsdatum";
    m_file << "Geschlecht";
    m_file << "AlterBeiDiagnose";
    m_file << "Erstdiagnose";
    m_file << "T";
    m_file << "N";
    m_file << "M";
    m_file << "ROS1_IHC";
    m_file << "ROS1_pos";
    m_file << "ROS1_FISH_ratio";
    m_file << "EGFR";
    m_file << "KRAS";
    m_file << "PIK3CA";
    m_file << "BRAF";
    m_file << "ALK_pos";
    m_file << "pERK";
    m_file << "pAKT";
    m_file << "PTEN";
    m_file << "PTEN_pos";
    m_file << "HER2_IHC";
    m_file << "HER2_pos.";
    m_file << "HER2_DAKO";
    m_file << "HER2_FISH";
    m_file << "MET_overexp";
    m_file << "MET_Hscore";
    m_file << "MET_perc_weak";
    m_file << "MET_perc_medium";
    m_file << "MET_perc_strong";
    m_file << "OS";
    m_file << "OSerreicht";
    m_file << "Anz_Therapielinien";
    for (int i=0; i<reportedLines; i++)
    {
        m_file << QString("TTF") + QString::number(i+1);
        m_file << QString("TTF") + QString::number(i+1) + QString("erreicht");
    }

    enum SpecificTherapy
    {
        Pemetrexed,
        Platinum,
        Taxan,

        FirstSpecificTherapy = Pemetrexed,
        LastSpecificTherapy  = Taxan
    };

    QMultiMap<SpecificTherapy, QString> specificTherapySubstances;
    specificTherapySubstances.insert(Pemetrexed, "Pemetrexed");
    specificTherapySubstances.insert(Platinum, "Cisplatin");
    specificTherapySubstances.insert(Platinum, "Carboplatin");
    specificTherapySubstances.insert(Taxan, "Paclitaxel");
    specificTherapySubstances.insert(Taxan, "Docetaxel");

    QMap<SpecificTherapy, QString> specificTherapyShortcuts;
    specificTherapyShortcuts.insert(Pemetrexed, "Pem");
    specificTherapyShortcuts.insert(Platinum, "Platin");
    specificTherapyShortcuts.insert(Taxan, "Taxan");

    for (int i=FirstSpecificTherapy; i<=LastSpecificTherapy; ++i)
    {
        SpecificTherapy specificTherapy = SpecificTherapy(i);
        const QString& shortCut = specificTherapyShortcuts.value(specificTherapy);
        m_file << shortCut + "_line_number";
        m_file << "OS_" + shortCut;
        m_file << "TTF_" + shortCut;
        m_file << "TTF_" + shortCut + "_erreicht";
    }

    m_file.newLine();

    QList<Patient::Ptr> patients;
    patients += ros1patients;
    const int size = models.filterModel()->rowCount();
    for (int i=0; i<size; i++)
    {
        Patient::Ptr p = PatientModel::retrievePatient(models.filterModel()->index(i, 0));
        if (!ros1patients.contains(p))
        {
            patients << p;
        }
    }

    foreach (const Patient::Ptr& p, patients)
    {
        m_currentPatient = p;
        const Disease& disease = p->firstDisease();
        const DiseaseHistory& history = disease.history;

        if (p->surname.contains("Dktk"))
        {
            continue;
        }

        /// Metadata
        m_file << p->id;
        /*m_file << p->surname;
        m_file << p->firstName;
        m_file << p->dateOfBirth;*/
        m_file << p->surname.right(1);
        m_file << p->firstName.right(1);
        m_file << QVariant();
        m_file << (p->gender == Patient::Male ? 1 : 0);
        m_file << (p->dateOfBirth.daysTo(disease.initialDiagnosis) / 365.0);
        m_file << disease.initialDiagnosis;
        m_file << disease.initialTNM.Tnumber();
        m_file << disease.initialTNM.Nnumber();
        TNM::MStatus m = disease.initialTNM.mstatus();
        if (m == TNM::Mx)
        {
            //qDebug() << "Mx status for" << p->surname << p->firstName << disease.initialTNM.toText();
        }
        if (disease.initialTNM.toText().contains("Mx", Qt::CaseInsensitive))
        {
            //qDebug() << "Real Mx status for" << p->surname << p->firstName << disease.initialTNM.toText();
        }
        m_file << (m == TNM::Mx ? QVariant() : QVariant(int(m)));

        writePathologyProperty(disease, PathologyPropertyInfo::IHC_ROS1);
        writePathologyProperty(disease, PathologyPropertyInfo::Fish_ROS1);
        writeDetailValue(disease, PathologyPropertyInfo::Fish_ROS1);

        writePathologyProperty(disease, PathologyPropertyInfo::Mut_EGFR_19_21);
        writePathologyProperty(disease, PathologyPropertyInfo::Mut_KRAS_2);
        writePathologyProperty(disease, PathologyPropertyInfo::Mut_PIK3CA_10_21);
        writePathologyProperty(disease, PathologyPropertyInfo::Mut_BRAF_15);
        writePathologyProperty(disease, PathologyPropertyInfo::Fish_ALK);
        writePathologyProperty(disease, PathologyPropertyInfo::IHC_pERK);
        writePathologyProperty(disease, PathologyPropertyInfo::IHC_pAKT);
        writePathologyProperty(disease, PathologyPropertyInfo::IHC_PTEN);
        writeIHCIsPositive(disease, PathologyPropertyInfo::IHC_PTEN);

        writePathologyProperty(disease, PathologyPropertyInfo::IHC_HER2);
        CombinedValue her2comb(PathologyPropertyInfo::Comb_HER2);
        her2comb.combine(disease);
        m_file << her2comb.toValue();
        QVariant her2Dako = writePathologyProperty(disease, PathologyPropertyInfo::IHC_HER2_DAKO);
        m_file << her2comb.fishResult(disease);

        CombinedValue metComb(PathologyPropertyInfo::Comb_cMetActivation);
        metComb.combine(disease);
        m_file << metComb.toValue();
        writePathologyProperty(disease, PathologyPropertyInfo::IHC_cMET);

        PathologyPropertyInfo infoMet(PathologyPropertyInfo::IHC_cMET);
        ValueTypeCategoryInfo ihcTypeMet(PathologyPropertyInfo::IHC_cMET);
        Property metProp = disease.pathologyProperty(infoMet.id);
        if (metProp.isValid())
        {
            HScore hscore = ihcTypeMet.toMedicalValue(metProp).value<HScore>();
            m_file << hscore.percentageWeak();
            m_file << hscore.percentageMedium();
            m_file << hscore.percentageStrong();
        }
        else
        {
            m_file << QVariant() << QVariant() << QVariant();
        }

        /// OS

        OSIterator osit(disease);
        osit.setProofreader(this);
        if (!history.isEmpty())
        {
            m_file << osit.days(OSIterator::FromFirstTherapy);
            m_file << (int)osit.endpointReached();
        }
        else
        {
            m_file << QVariant();
            m_file << QVariant();
        }

        NewTreatmentLineIterator treatmentLinesIterator;
        treatmentLinesIterator.set(history);
        treatmentLinesIterator.iterateToEnd();
        QList<TherapyGroup> therapies = treatmentLinesIterator.therapies();
        QDate lastEndDate = history.begin();
        QList<QDate> lineDates, ctxLineDates;
        QVector<HistoryElement*> firstTherapies(LastSpecificTherapy+1);
        QVector<int> firstTherapyLines(LastSpecificTherapy+1);
        foreach (const TherapyGroup& group, therapies)
        {
            if (group.hasChemotherapy())
            {
                ctxLineDates << group.beginDate();
            }

            for (int i=FirstSpecificTherapy; i<=LastSpecificTherapy; ++i)
            {
                SpecificTherapy specificTherapy = SpecificTherapy(i);

                // already found a first line? Continue
                if (firstTherapies[specificTherapy])
                {
                    continue;
                }

                // Try all possible substances, check if this line contains the substance
                foreach (const QString& substance, specificTherapySubstances.values(specificTherapy))
                {
                    if (group.hasSubstance(substance))
                    {
                        foreach (Therapy*t, group)
                        {
                            if (t->elements.hasSubstance(substance))
                            {
                                firstTherapies[specificTherapy] = t;
                                break;
                            }
                        }
                        firstTherapyLines[specificTherapy] = ctxLineDates.size() - 1;
                        break;
                    }
                }
            }

            // skip groups fully contained in another group
            if (group.effectiveEndDate() > lastEndDate || lastEndDate == history.begin())
            {
                lineDates << group.beginDate();
            }
            lastEndDate = group.effectiveEndDate();
        }
        if (history.isEmpty())
        {
            m_file << QVariant();
        }
        else
        {
            m_file << ctxLineDates.size();
        }

        CurrentStateIterator currentStateIterator(history);
        currentStateIterator.setProofreader(this);
        QList<int> ttfList; QList<bool> ttfEndpointReachedList;
        for (int line = 0; line<ctxLineDates.size(); line++)
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
            ttfList << begin.daysTo(end);
            ttfEndpointReachedList << reachedEndpoint;
        }
        for (int line=0; line < reportedLines; line++)
        {
            if (line < ttfList.size())
            {
                m_file << ttfList[line];
                m_file << ttfEndpointReachedList[line];
            }
            else
            {
                m_file << QVariant();
                m_file << QVariant();
            }
        }

        for (int i=FirstSpecificTherapy; i<=LastSpecificTherapy; ++i)
        {
            SpecificTherapy specificTherapy = SpecificTherapy(i);

            // Did we see a therapy line with that substance?
            if (firstTherapies[specificTherapy])
            {
                // number of first line with this substance
                m_file << firstTherapyLines[specificTherapy] + 1; // index is 0-based, line number is one-based
                // OS
                m_file << osit.days(firstTherapies[specificTherapy]);
                // TTF
                reportTTF(ctxLineDates, osit, firstTherapyLines[specificTherapy], firstTherapies[specificTherapy]->date);
            }
            else
            {
                m_file << QVariant();
                m_file << QVariant();
                m_file << QVariant();
                m_file << QVariant();
                continue;
            }

        }

        m_file.newLine();
    }

    m_file.finishWriting();
}

void AnalysisGenerator::listsForRadiologyProject()
{
    PatientPropertyModelViewAdapter models;
    models.setReportType(PatientPropertyModelViewAdapter::PulmonaryAdenoIHCMut);

    m_file.openForWriting("/home/marcel/Dokumente/Tumorprofil/Radiologie-Projekt Simon/Liste Simon Adeno EGFR vs pan-WT.csv");
    m_file << "id";
    m_file << "Nachname";
    m_file << "Vorname";
    m_file << "Geburtsdatum";
    m_file << "Erstdiagnose";
    m_file << "pan-WT vs. EGFR";
    m_file << "EGFR Mutation";
    m_file.newLine();

    int size = models.filterModel()->rowCount();
    for (int i=0; i<size; i++)
    {
        Patient::Ptr p = PatientModel::retrievePatient(models.filterModel()->index(i, 0));
        if (p->surname.contains("Dktk"))
        {
            continue;
        }
        m_currentPatient = p;
        const Disease& disease = p->firstDisease();

        ActionableResultChecker actionableResults(p, ActionableResultChecker::IncludeRAS);
        QList<PathologyPropertyInfo> results = actionableResults.actionableResults();
        results.removeAll(PathologyPropertyInfo::info(PathologyPropertyInfo::Comb_cMetActivation));
        results.removeAll(PathologyPropertyInfo::info(PathologyPropertyInfo::Comb_HER2));

        bool egfr = results.contains(PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_EGFR_19_21));
        if (!egfr && !results.isEmpty())
        {
            continue;
        }
        if (!disease.hasPathologyProperty(PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_KRAS_2).id))
        {
            continue; // require at least KRAS Exon 2 to come into the "wild-type" list
        }

        m_file << p->id;
        m_file << p->surname;
        m_file << p->firstName;
        m_file << p->dateOfBirth;
        m_file << disease.initialDiagnosis;
        m_file << egfr;
        if (egfr)
        {
            writeDetailValue(disease, PathologyPropertyInfo::Mut_EGFR_19_21);
        }
        else
        {
            m_file << QVariant();
        }

        m_file.newLine();
    }

    m_file.finishWriting();
    models.setReportType(PatientPropertyModelViewAdapter::CRCIHCMut);

    m_file.openForWriting("/home/marcel/Dokumente/Tumorprofil/Radiologie-Projekt Simon/Liste Simon CRC RAS-mut pan-WT.csv");
    m_file << "id";
    m_file << "Nachname";
    m_file << "Vorname";
    m_file << "Geburtsdatum";
    m_file << "Erstdiagnose";
    m_file << "pan-WT vs. RAS";
    m_file << "RAS Protein";
    m_file << "RAS Mutation";
    m_file << "Habe Negatives KRAS34";
    m_file << "Habe negatives NRAS";
    m_file << "Habe negatives KRAS-NRAS";
    m_file.newLine();

    size = models.filterModel()->rowCount();
    for (int i=0; i<size; i++)
    {
        Patient::Ptr p = PatientModel::retrievePatient(models.filterModel()->index(i, 0));
        m_currentPatient = p;
        const Disease& disease = p->firstDisease();
        CombinedValue comb(PathologyPropertyInfo::info(PathologyPropertyInfo::Comb_RASMutation));
        comb.combine(disease);
        bool ras = comb.toValue().toBool();
        if (!ras)
        {
            if (!disease.hasPathologyProperty(PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_KRAS_2).id))
            {
                continue; // require at least KRAS Exon 2 to come into the "wild-type" list
            }

            ActionableResultChecker actionableResults(p);
            QList<PathologyPropertyInfo> results = actionableResults.actionableResults();
            results.removeAll(PathologyPropertyInfo::info(PathologyPropertyInfo::Comb_cMetActivation));
            results.removeAll(PathologyPropertyInfo::info(PathologyPropertyInfo::Comb_HER2));
            if (!results.isEmpty())
            {
                continue;
            }
        }

        m_file << p->id;
        m_file << p->surname;
        m_file << p->firstName;
        m_file << p->dateOfBirth;
        m_file << disease.initialDiagnosis;
        m_file << ras;
        if (ras)
        {
            m_file << PathologyPropertyInfo::info(comb.originalProperty().property).label;
            m_file << comb.originalProperty().detail;
            m_file << QVariant() << QVariant() << QVariant(); // negative RAS n/a
        }
        else
        {
            m_file << QVariant();
            m_file << QVariant();
            bool hasNRAS = disease.hasPathologyProperty(PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_NRAS_2_4).id);
            bool hasKRAS = (disease.hasPathologyProperty(PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_KRAS_3).id)
                            && disease.hasPathologyProperty(PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_KRAS_4).id));
            m_file << hasKRAS;
            m_file << hasNRAS;
            m_file << (hasKRAS && hasNRAS);
        }

        m_file.newLine();
    }

    m_file.finishWriting();
}

void AnalysisGenerator::cmetListFromCSV()
{
    QList<Patient::Ptr> patients = patientsFromCSV("/home/marcel/Dokumente/Tumorprofil/cMET-Projekt/Patientenliste.csv");
    m_file.openForWriting("/home/marcel/Dokumente/Tumorprofil/cMET-Projekt/Patienten klinische Daten.csv");
    m_file << "id";
    m_file << "Nachname";
    m_file << "Vorname";
    m_file << "Geburtsdatum";
    m_file << "OS";
    m_file << "OSerreicht";
    m_file.newLine();

    qDebug() << "Identified" << patients.size();

    foreach (Patient::Ptr p, patients)
    {
        m_file << p->id;
        m_file << p->surname;
        m_file << p->firstName;
        m_file << p->dateOfBirth;

        const Disease& disease = p->firstDisease();
        const DiseaseHistory& history = disease.history;

        if (!history.isEmpty())
        {
            OSIterator it(disease);
            it.setProofreader(this);
            m_file << it.days(OSIterator::FromFirstTherapy);
            m_file << (int)it.endpointReached();
        }
        else
        {
            m_file << QVariant() << QVariant();
        }

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
