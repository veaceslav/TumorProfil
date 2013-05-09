/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 31.01.2012
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

#include "pathologywidgetgenerator.h"

// Qt includes

#include <QFormLayout>
#include <QDebug>
#include <QLabel>

// Local includes

#include "pathologypropertywidget.h"

PathologyWidgetGenerator::PathologyWidgetGenerator()
    : m_entity(Pathology::UnknownEntity),
      m_context(PathologyContextInfo::Tumorprofil)
{
}

PathologyWidgetGenerator::~PathologyWidgetGenerator()
{
    clear();
}

void PathologyWidgetGenerator::clear()
{
    qDeleteAll(m_objects);
    m_objects.clear();
    m_widgets.clear();
    m_hash.clear();
}

bool PathologyWidgetGenerator::switchEntity(Pathology::Entity entity, PathologyContextInfo::Context context)
{
    if (entity == m_entity && context == m_context)
    {
        return false;
    }
    clear();
    createWidgets(entity, context);
    return true;
}

bool PathologyWidgetGenerator::switchEntity(Pathology::Entity entity, PathologyContextInfo::Context context,
                                            QFormLayout* layout)
{
    if (switchEntity(entity, context))
    {
        addWidgetsToLayout(layout);
        return true;
    }
    return false;
}

PathologyPropertyWidget* PathologyWidgetGenerator::create(PathologyPropertyInfo::Property property)
{
    PathologyPropertyInfo info = PathologyPropertyInfo::info(property);
    PathologyPropertyWidget* w = new PathologyPropertyWidget(info);
    w->setLabel(info.label);
    w->setDetailLabel(info.detailLabel);
    return w;
}

QList<QObject*> PathologyWidgetGenerator::createWidgets(Pathology::Entity e, PathologyContextInfo::Context context)
{   
    m_entity = e;
    m_context = context;
    switch (m_context)
    {
    case PathologyContextInfo::Tumorprofil:

        switch (m_entity)
        {
        case Pathology::PulmonaryAdeno:
        case Pathology::PulmonaryBronchoalveloar:
        case Pathology::PulmonaryLargeCell:
        case Pathology::PulmonaryAdenosquamous:
        case Pathology::PulmonaryOtherCarcinoma:
            m_objects << new QLabel(tr("Immunhistochemie"));
            m_objects << create(PathologyPropertyInfo::IHC_PTEN);
            m_objects << create(PathologyPropertyInfo::IHC_pAKT);
            m_objects << create(PathologyPropertyInfo::IHC_pERK);
            m_objects << create(PathologyPropertyInfo::IHC_ALK);
            m_objects << create(PathologyPropertyInfo::IHC_HER2);
            m_objects << create(PathologyPropertyInfo::IHC_HER2_DAKO);
            m_objects << create(PathologyPropertyInfo::IHC_cMET);

            m_objects << new QLabel(tr("FISH"));
            m_objects << create(PathologyPropertyInfo::Fish_ALK);
            m_objects << create(PathologyPropertyInfo::Fish_HER2);
            m_objects << create(PathologyPropertyInfo::Fish_cMET);

            m_objects << new QLabel(tr("Sequenzanalysen"));
            m_objects << create(PathologyPropertyInfo::Mut_KRAS_2);
            m_objects << create(PathologyPropertyInfo::Mut_EGFR_19_21);
            m_objects << create(PathologyPropertyInfo::Mut_PIK3CA_10_21);
            m_objects << create(PathologyPropertyInfo::Mut_BRAF_15);
            m_objects << new QLabel(tr("Weitere Sequenzanalysen"));
            m_objects << create(PathologyPropertyInfo::Mut_EGFR_18_20);
            m_objects << create(PathologyPropertyInfo::Mut_KRAS_3);
            m_objects << create(PathologyPropertyInfo::Mut_BRAF_11);
            break;
        case Pathology::PulmonarySquamous:
            m_objects << new QLabel(tr("Immunhistochemie"));
            m_objects << create(PathologyPropertyInfo::IHC_PTEN);
            m_objects << create(PathologyPropertyInfo::IHC_pAKT);
            m_objects << create(PathologyPropertyInfo::IHC_pERK);

            m_objects << new QLabel(tr("FISH"));
            m_objects << create(PathologyPropertyInfo::Fish_FGFR1);
            m_objects << create(PathologyPropertyInfo::Fish_PIK3CA);
            m_objects << create(PathologyPropertyInfo::Fish_cMET);

            m_objects << new QLabel(tr("Sequenzanalysen"));
            m_objects << create(PathologyPropertyInfo::Mut_PIK3CA_10_21);
            m_objects << create(PathologyPropertyInfo::Mut_DDR2);

            m_objects << new QLabel(tr("Weitere Sequenzanalysen (Adenosquamös)"));
            m_objects << create(PathologyPropertyInfo::Mut_EGFR_19_21);
            m_objects << create(PathologyPropertyInfo::Mut_KRAS_2);
            m_objects << create(PathologyPropertyInfo::Mut_KRAS_3);
            m_objects << create(PathologyPropertyInfo::Mut_BRAF_15);
            break;
        case Pathology::ColorectalAdeno:
            m_objects << new QLabel(tr("Immunhistochemie"));
            m_objects << create(PathologyPropertyInfo::IHC_pAKT);
            m_objects << create(PathologyPropertyInfo::IHC_pP70S6K);
            m_objects << create(PathologyPropertyInfo::IHC_pERK);
            m_objects << create(PathologyPropertyInfo::IHC_PTEN);
            m_objects << create(PathologyPropertyInfo::IHC_cMET);

            m_objects << new QLabel(tr("Sequenzanalysen"));
            m_objects << create(PathologyPropertyInfo::Mut_KRAS_2);
            m_objects << create(PathologyPropertyInfo::Mut_PIK3CA_10_21);
            m_objects << new QLabel(tr("Sequenzanalysen k-ras Exon 2 Wildtyp"));
            m_objects << create(PathologyPropertyInfo::Mut_KRAS_3);
            m_objects << create(PathologyPropertyInfo::Mut_BRAF_15);

            m_objects << new QLabel(tr("Sonderuntersuchungen (V.a. HNPCC)"));
            m_objects << create(PathologyPropertyInfo::IHC_MLH1);
            m_objects << create(PathologyPropertyInfo::IHC_MSH2);
            m_objects << create(PathologyPropertyInfo::IHC_MSH6);
            m_objects << create(PathologyPropertyInfo::PCR_D5S346);
            m_objects << create(PathologyPropertyInfo::PCR_BAT26);
            m_objects << create(PathologyPropertyInfo::PCR_BAT25);
            m_objects << create(PathologyPropertyInfo::PCR_D17S250);
            m_objects << create(PathologyPropertyInfo::PCR_D2S123);
            break;
        case Pathology::RenalCell:
            m_objects << new QLabel(tr("Sequenzanalysen"));
            m_objects << create(PathologyPropertyInfo::Mut_PIK3CA_10_21);
            break;
        case Pathology::Cholangiocarcinoma:
            m_objects << new QLabel(tr("Sequenzanalysen"));
            m_objects << create(PathologyPropertyInfo::Mut_KRAS_2);
            break;
        case Pathology::Esophageal:
        case Pathology::EsophagogastrealJunction:
        case Pathology::Gastric:
            m_objects << new QLabel(tr("Immunhistochemie"));
            m_objects << create(PathologyPropertyInfo::IHC_pAKT);
            m_objects << create(PathologyPropertyInfo::IHC_pP70S6K);
            m_objects << create(PathologyPropertyInfo::IHC_pERK);
            m_objects << create(PathologyPropertyInfo::IHC_PTEN);
            m_objects << new QLabel(tr("Sequenzanalysen"));
            m_objects << create(PathologyPropertyInfo::Mut_PIK3CA_10_21);
            break;
        case Pathology::Breast:
            m_objects << new QLabel(tr("Immunhistochemie"));
            m_objects << create(PathologyPropertyInfo::IHC_PTEN);
            m_objects << create(PathologyPropertyInfo::IHC_pAKT);
            m_objects << create(PathologyPropertyInfo::IHC_pP70S6K);
            m_objects << create(PathologyPropertyInfo::IHC_pERK);
            m_objects << new QLabel(tr("Sequenzanalysen"));
            m_objects << create(PathologyPropertyInfo::Mut_PIK3CA_10_21);
            m_objects << new QLabel(tr("Rezeptorstatus"));
            m_objects << create(PathologyPropertyInfo::IHC_ER);
            m_objects << create(PathologyPropertyInfo::IHC_PR);
            m_objects << create(PathologyPropertyInfo::IHC_HER2_DAKO);
            m_objects << new QLabel(tr("FISH (je nach Her2-Status)"));
            m_objects << create(PathologyPropertyInfo::Fish_HER2);
            m_objects << new QLabel(tr("FISH"));
            m_objects << create(PathologyPropertyInfo::Fish_FGFR1);
        case Pathology::TransitionalCell:
        case Pathology::Thyroid:
        case Pathology::Melanoma:
        case Pathology::UnknownEntity:
            break;
        }
        break;

    case PathologyContextInfo::ScreeningBGJ398:
        m_objects << new QLabel(tr("FISH"));
        m_objects << create(PathologyPropertyInfo::Fish_FGFR1);
        break;
    case PathologyContextInfo::ScreeningBEZ235:
        m_objects << new QLabel(tr("Sequenzanalysen"));
        m_objects << create(PathologyPropertyInfo::Mut_PIK3CA_10_21);
        m_objects << new QLabel(tr("Immunhistochemie"));
        m_objects << create(PathologyPropertyInfo::IHC_PTEN);
        break;
    case PathologyContextInfo::ScreeningBKM120:
        m_objects << new QLabel(tr("Sequenzanalysen"));
        m_objects << create(PathologyPropertyInfo::Mut_PIK3CA_10_21);
        m_objects << create(PathologyPropertyInfo::Mut_PTEN);
        m_objects << new QLabel(tr("Immunhistochemie"));
        m_objects << create(PathologyPropertyInfo::IHC_PTEN);
        break;
    case PathologyContextInfo::BestRx:
        m_objects << new QLabel(tr("PI3K/Akt-Signalweg"));
        m_objects << create(PathologyPropertyInfo::Mut_PIK3CA_10_21);
        m_objects << create(PathologyPropertyInfo::Fish_PIK3CA);
        m_objects << create(PathologyPropertyInfo::IHC_PTEN);
        m_objects << new QLabel(tr("FGFR1"));
        m_objects << create(PathologyPropertyInfo::Fish_FGFR1);
        m_objects << new QLabel(tr("Hormonstatus"));
        m_objects << create(PathologyPropertyInfo::IHC_ER);
        m_objects << create(PathologyPropertyInfo::IHC_PR);
        m_objects << create(PathologyPropertyInfo::IHC_HER2_DAKO);
        m_objects << create(PathologyPropertyInfo::Fish_HER2);
        break;
    default:
        break;

    }


    foreach (QObject* o, m_objects)
    {
        if (o->inherits("PathologyPropertyWidget"))
        {
            PathologyPropertyWidget* w = static_cast<PathologyPropertyWidget*>(o);
            m_widgets << w;
            m_hash[w->propertyName()] = w;
        }
    }
    return m_objects;
}

PathologyPropertyWidget* PathologyWidgetGenerator::propertyWidget(const QString& property) const
{
    return m_hash.value(property);
}

void PathologyWidgetGenerator::addWidgetsToLayout(QFormLayout* layout)
{
    foreach (QObject* o, m_objects)
    {
        if (o->isWidgetType())
        {
            QWidget* w = static_cast<QWidget*>(o);
            layout->addRow(w);
        }
        else
        {
            PathologyPropertyWidget* w = static_cast<PathologyPropertyWidget*>(o);
            w->addToLayout(layout);
        }
    }
}
