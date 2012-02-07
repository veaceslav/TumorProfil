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
    : m_entity(Pathology::UnknownEntity)
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

bool PathologyWidgetGenerator::switchEntity(Pathology::Entity entity)
{
    if (entity == m_entity)
    {
        return false;
    }
    clear();
    createWidgets(entity);
    return true;
}

bool PathologyWidgetGenerator::switchEntity(Pathology::Entity entity, QFormLayout* layout)
{
    if (switchEntity(entity))
    {
        addWidgetsToLayout(layout);
        return true;
    }
    return false;
}

QList<QObject*> PathologyWidgetGenerator::createWidgets(Pathology::Entity e)
{   
    m_entity = e;
    switch (m_entity)
    {
    case Pathology::PulmonaryAdeno:
    case Pathology::PulmonaryBronchoalveloar:
    case Pathology::PulmonaryLargeCell:
        m_objects << new QLabel(tr("Immunhistochemie"));
        m_objects << PathologyPropertyWidget::createIHCBoolean("ihc/pten", tr("PTEN"));
        m_objects << PathologyPropertyWidget::createIHC("ihc/p-akt?p=s473", tr("<qt>p-AKT<sup>S473</sup></qt>"));
        m_objects << PathologyPropertyWidget::createIHC("ihc/p-erk", tr("p-ERK"));
        m_objects << PathologyPropertyWidget::createIHC("ihc/alk", tr("ALK"));
        m_objects << PathologyPropertyWidget::createIHC("ihc/her2", tr("HER2"));

        m_objects << new QLabel(tr("FISH"));
        m_objects << PathologyPropertyWidget::createFish("fish/alk", tr("ALK-Translokation"), tr("Prozentsatz:"));
        m_objects << PathologyPropertyWidget::createFish("fish/her2", tr("HER2-Amplifikation"), tr("Ratio HER2/CEP7:"));

        m_objects << new QLabel(tr("Sequenzanalysen"));
        m_objects << PathologyPropertyWidget::createMutation("mut/kras?exon=2", tr("KRAS Exon 2"));
        m_objects << PathologyPropertyWidget::createMutation("mut/egfr?exon=19,21", tr("EGFR Exon 19 & 21"));
        m_objects << PathologyPropertyWidget::createMutation("mut/pik3ca?exon=10,21", tr("PIK3CA Exon 10 & 21"));
        m_objects << PathologyPropertyWidget::createMutation("mut/braf?exon=15", tr("BRAF Exon 15"));
        m_objects << new QLabel(tr("Weitere Sequenzanalysen"));
        m_objects << PathologyPropertyWidget::createMutation("mut/egfr?exon=18,20", tr("EGFR Exon 18 & 20"));
        m_objects << PathologyPropertyWidget::createMutation("mut/kras?exon=3", tr("KRAS Exon 3"));
        m_objects << PathologyPropertyWidget::createMutation("mut/braf?exon=11", tr("BRAF Exon 11"));
        break;
    case Pathology::PulmonarySquamous:
    case Pathology::PulmonaryAdenosquamous:
        m_objects << new QLabel(tr("Immunhistochemie"));
        m_objects << PathologyPropertyWidget::createIHCBoolean("ihc/pten", tr("PTEN"));
        m_objects << PathologyPropertyWidget::createIHC("ihc/p-akt?p=s473", tr("<qt>p-AKT<sup>S473</sup></qt>"));
        m_objects << PathologyPropertyWidget::createIHC("ihc/p-erk", tr("p-ERK"));

        m_objects << new QLabel(tr("FISH"));
        m_objects << PathologyPropertyWidget::createFish("fish/fgfr1", tr("FGFR1-Amplifikation"), tr("Ratio FGFR1/CEP8:"));

        m_objects << new QLabel(tr("Sequenzanalysen"));
        m_objects << PathologyPropertyWidget::createMutation("mut/pik3ca?exon=10,21", tr("PIK3CA Exon 10 & 21"));
        m_objects << PathologyPropertyWidget::createMutation("mut/ddr2?exon=15-18", tr("DDR2 Exon 15-18"));

        m_objects << new QLabel(tr("Weitere Sequenzanalysen (Adenosquamös)"));
        m_objects << PathologyPropertyWidget::createMutation("mut/egfr?exon=19,21", tr("EGFR Exon 19 & 21"));
        m_objects << PathologyPropertyWidget::createMutation("mut/kras?exon=2", tr("KRAS Exon 2"));
        m_objects << PathologyPropertyWidget::createMutation("mut/kras?exon=3", tr("KRAS Exon 3"));
        m_objects << PathologyPropertyWidget::createMutation("mut/braf?exon=15", tr("BRAF Exon 15"));
        break;
    case Pathology::ColorectalAdeno:
        m_objects << new QLabel(tr("Immunhistochemie"));
        m_objects << PathologyPropertyWidget::createIHC("ihc/p-akt?p=s473", tr("<qt>p-AKT<sup>S473</sup></qt>"));
        m_objects << PathologyPropertyWidget::createIHCBooleanPercentage("ihc/p-p70S6k", tr("p-p70S6K"));
        m_objects << PathologyPropertyWidget::createIHCBooleanPercentage("ihc/p-erk", tr("p-ERK"));
        m_objects << PathologyPropertyWidget::createIHCBoolean("ihc/pten", tr("PTEN"));

        m_objects << new QLabel(tr("Sequenzanalysen"));
        m_objects << PathologyPropertyWidget::createMutation("mut/kras?exon=2", tr("KRAS Exon 2"));
        m_objects << PathologyPropertyWidget::createMutation("mut/pik3ca?exon=10,21", tr("PIK3CA Exon 10 & 21"));
        m_objects << new QLabel(tr("Sequenzanalysen k-ras Exon 2 Wildtyp"));
        m_objects << PathologyPropertyWidget::createMutation("mut/kras?exon=3", tr("KRAS Exon 3"));
        m_objects << PathologyPropertyWidget::createMutation("mut/braf?exon=15", tr("BRAF Exon 15"));

        m_objects << new QLabel(tr("Sonderuntersuchungen (V.a. HNPCC)"));
        m_objects << PathologyPropertyWidget::createIHC("ihc/mlh1", tr("MLH1"));
        m_objects << PathologyPropertyWidget::createIHC("ihc/msh2", tr("MSH2"));
        m_objects << PathologyPropertyWidget::createIHC("ihc/msh6", tr("MSH6"));
        m_objects << PathologyPropertyWidget::createStableUnstable("msi/d5s346", tr("MSI PCR: D5S346"));
        m_objects << PathologyPropertyWidget::createStableUnstable("msi/bat26", tr("MSI PCR: BAT26"));
        m_objects << PathologyPropertyWidget::createStableUnstable("msi/bat25", tr("MSI PCR: BAT25"));
        m_objects << PathologyPropertyWidget::createStableUnstable("msi/d17s250", tr("MSI PCR: D17S250"));
        m_objects << PathologyPropertyWidget::createStableUnstable("msi/d2s123", tr("MSI PCR: D2S123"));
        break;
    case Pathology::UnknownEntity:
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
