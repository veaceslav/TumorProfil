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

void AnalysisGenerator::her2()
{
    PatientPropertyModelViewAdapter models;
    models.setReportType(PatientPropertyModelViewAdapter::PulmonaryAdenoIHCMut);

    m_file.openForWriting("C:\\Users\\wiesweg\\Documents\\Tumorprofil\\HER2-Auswertung 14032014.csv");

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
    m_file.newLine();

    const int size = models.filterModel()->rowCount();
    for (int i=0; i<size; i++)
    {
        Patient::Ptr p = PatientModel::retrievePatient(models.filterModel()->index(i, 0));
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
        m_file << int(m); // 4

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
        m_file << it.days(OSIterator::FromFirstTherapy); // 19
        m_file << (int)it.endpointReached(); // 20

        m_file.newLine();
    }

    m_file.finishWriting();
}
