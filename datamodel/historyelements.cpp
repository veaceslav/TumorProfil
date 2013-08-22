/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 14.02.2013
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

#include "historyelements.h"

HistoryElement::HistoryElement()
    : m_parent(0)
{
}

HistoryElement::~HistoryElement()
{
}

HistoryElement*HistoryElement:: parent() const
{
    return m_parent;
}

void HistoryElement::setParent(HistoryElement* parent)
{
    m_parent = parent;
}

Chemotherapy::Chemotherapy()
    : dose(0), absdose(0)
{
}

QStringList Chemotherapy::substances()
{
    QStringList list;
    list << "Cisplatin"
         << "Carboplatin"
         << "Oxaliplatin"
         << "Paclitaxel"
         << "Docetaxel"
         << "Irinotecan"
         << "Topotecan"
         << "Vincristin"
         << "Vinorelbin"
         << "Vinorelbin p.o."
         << "5-FU"
         << "Leucovorin"
         << "Capecitabin"
         << "Pemetrexed"
         << "Bevacizumab"
         << "Cetuximab"
         << "Aflibercept"
         << "Gemcitabin"
         << "Etoposid"
         << "Doxorubicin"
         << "Epirubicin"
         << "Ifosfamid"
         << "Cyclophosphamid"
         << "Abraxane"
         << "Erlotinib"
         << "Gefitinib"
         << "Afatinib"
         << "Regorafenib"
         << "Trastuzumab";
    return list;
}

Radiotherapy::Radiotherapy()
    : dose(0)
{
}

Toxicity::Toxicity()
    : grade(0)
{
}

TherapyElementList::TherapyElementList(HistoryElement* parent)
    : GenericElementList<TherapyElement>(parent)
{
}

QStringList TherapyElementList::substances() const
{
    QStringList substances;
    foreach (const Chemotherapy* ctx, filtered<Chemotherapy>())
    {
        substances << ctx->substance;
    }
    /*foreach (TherapyElement* elem, *this)
    {
        Chemotherapy* ctx = dynamic_cast<Chemotherapy*>(elem);
        if (elem)
        {
            substances << ctx->substance;
        }
    }*/
    return substances;
}

TherapyElementList& TherapyElementList::operator<<(TherapyElement* elem)
{
    elem->setParent(m_parent);
    append(elem);
    return *this;
}


Therapy::Therapy()
    : elements(this)
{
}

Therapy::~Therapy()
{
    qDeleteAll(elements);
}

QString Therapy::uiLabel(Type type)
{
    switch (type)
    {
    case Therapy::CTx:
        return QObject::tr("Chemotherapie");
    case Therapy::RTx:
        return QObject::tr("Radiotherapie");
    case Therapy::RCTx:
        return QObject::tr("Radiochemotherapie");
    case Therapy::Surgery:
        return QObject::tr("Operation");
    case Therapy::Intervention:
        return QObject::tr("Intervention");
    }
    return QString();
}

Finding::Finding()
    : type(UndefinedType), result(UndefinedResult), context(UndefinedContext)
{
}

DiseaseState::DiseaseState()
    : state(UnknownState)
{
}
