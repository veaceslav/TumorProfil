/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 10.10.2013
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

#include "historyiterator.h"

// Qt includes

#include <QtAlgorithms>
#include <QDebug>

// Local includes


HistoryIterator::HistoryIterator()
    : m_state(Invalid)
{
}

void HistoryIterator::set(const DiseaseHistory& history, HistoryElement* e)
{
    if (!e)
    {
        set(history, history.entries().begin());
    }
    else
    {
        set(history, qFind(history.entries(), e));
    }
}

void HistoryIterator::set(const HistoryIterator& other)
{
    if (other.m_state == Invalid)
    {
        m_state   = Invalid;
        m_history = DiseaseHistory();
        m_current = HistoryElementList::const_iterator();
        return;
    }
    set(other.m_history, other.m_current);
}

void HistoryIterator::set(const DiseaseHistory& history, HistoryElementList::const_iterator it)
{
    m_state   = Initialized;
    m_history = history;
    m_current = it;
}

HistoryIterator::State HistoryIterator::start()
{
    if (m_state == Invalid)
    {
        return m_state;
    }
    if (m_current == m_history.entries().constEnd())
    {
        m_state = AtEnd;
        return m_state;
    }
    if (m_state == Initialized)
    {
        reinitialized();
    }
    restarting();
    iterate();
    return m_state;
}

HistoryIterator::State HistoryIterator::startNext()
{
    if (m_state == AtEnd || m_state == Invalid)
    {
        return m_state;
    }
    m_current++;
    return start();
}

HistoryIterator::State HistoryIterator::next()
{
    if (m_state == Initialized)
    {
        return start();
    }
    else
    {
        return startNext();
    }
}

HistoryIterator::State HistoryIterator::iterateToEnd()
{
    while (m_state != AtEnd && m_state != Invalid)
    {
        next();
    }
    return m_state;
}


HistoryIterator::State HistoryIterator::currentState() const
{
    return m_state;
}

HistoryElement* HistoryIterator::currentElement() const
{
    switch (m_state)
    {
    case Initialized:
    case Match:
        return *m_current;
    default:
        return 0;
    }
}

void HistoryIterator::restarting()
{
}

void HistoryIterator::reinitialized()
{
}


void HistoryIterator::iterate()
{
    for (;;++m_current)
    {
        if (m_current == m_history.entries().constEnd())
        {
            m_state = AtEnd;
            return;
        }
        if (isInterested(*m_current))
        {
            if (visit(*m_current))
            {
                m_state = Match;
                return;
            }
        }

    }
}

// -------------------------------------------------------

CurrentStateIterator::CurrentStateIterator() : lastState(0)
{
}

CurrentStateIterator::CurrentStateIterator(const Disease &disease) : lastState(0)
{
    set(disease.history());
    start();
}

DiseaseState::State CurrentStateIterator::state() const
{
    if (!lastState)
    {
        return DiseaseState::UnknownState;
    }
    return lastState->state;
}

bool CurrentStateIterator::isInterested(HistoryElement* element)
{
    return element->is<DiseaseState>();
}

void CurrentStateIterator::restarting()
{
    lastState = 0;
}

bool CurrentStateIterator::visit(HistoryElement* element)
{
    lastState = element->as<DiseaseState>();
    return false;
}

// --------------------------------------------------------

OSIterator::OSIterator(const Disease& disease)
    : initialDiagnosis(disease.initialDiagnosis),
      firstTherapy(0),
      lastElement(0),
      endpointElement(0)
{
    set(disease.history());
    start();
}

int OSIterator::days(Definition definition) const
{
    QDate begin;
    switch (definition)
    {
    case FromInitialDiagnosis:
        begin = initialDiagnosis;
        break;
    case FromFirstTherapy:
        if (firstTherapy)
        {
            begin = firstTherapy->begin();
        }
        break;
    }
    if (!begin.isValid())
    {
        qDebug() << "Invalid begin date" << definition << initialDiagnosis << firstTherapy;
        return -1;
    }

    QDate end;
    if (endpointElement)
    {
        end = endpointElement->date;
    }
    else if (lastElement)
    {
        end = lastElement->date;
    }
    else
    {
        qDebug() << "Empty history, no OS";
    }
    return begin.daysTo(end);
}

bool OSIterator::endpointReached() const
{
    return endpointElement;
}

float OSIterator::months(Definition definition) const
{
    int d = days(definition);
    if (d == -1)
    {
        return -1;
    }
    return float(d)/30.0;
}

bool OSIterator::isInterested(HistoryElement* element)
{
    return true;
}

bool OSIterator::visit(HistoryElement* element)
{
    if (!firstTherapy)
    {
        if (element->is<Therapy>())
        {
            firstTherapy = element->as<Therapy>();
        }
    }
    lastElement = element;
    if (!endpointElement)
    {
        if (element->is<Finding>())
        {
            if (element->as<Finding>()->type == Finding::Death)
            {
                endpointElement = element;
            }
        }
        else if (element->is<DiseaseState>())
        {
            if (element->as<DiseaseState>()->state == DiseaseState::Deceased)
            {
                endpointElement = element;
            }
        }
    }
    return false;
}

void OSIterator::restarting()
{
    firstTherapy = 0;
    lastElement = 0;
    endpointElement = 0;
}

// --------------------------------------------------------

EffectiveStateIterator::EffectiveStateIterator(const DiseaseHistory& history)
    : m_effectiveState(DiseaseState::UnknownState),
      m_definingElement(0)
{
    set(history);
}

bool EffectiveStateIterator::isInterested(HistoryElement* element)
{
    return true;
}

QDate EffectiveStateIterator::stateValidTo() const
{
    return m_stateValidTo;
}

bool EffectiveStateIterator::visit(HistoryElement* element)
{
    // treat as invariable
    if (m_effectiveState == DiseaseState::Deceased)
    {
        return false;
    }

    m_stateValidTo = QDate();
    m_definingElement = 0;
    if (element->is<DiseaseState>())
    {
        DiseaseState* state = element->as<DiseaseState>();
        m_effectiveState = state->state;
        m_definingElement = element;

        return true;
    }
    else if (element->is<Finding>())
    {
        /* Context
        UndefinedContext,
        Antecedent,
        InitialDiagnosis,
        ResponseEvaluation,
        FollowUp*/
        if (m_effectiveState < DiseaseState::InitialDiagnosis)
        {
            Finding* f = element->as<Finding>();
            if (f->context == Finding::InitialDiagnosis)
            {
                m_effectiveState = DiseaseState::InitialDiagnosis;
                m_definingElement = element;
                return true;
            }
        }
    }
    else // if (element->is<Therapy>())
    {
        Therapy* t = element->as<Therapy>();
        m_effectiveState = DiseaseState::Therapy;
        // ensure a single-date therapy has a valid end date
        m_stateValidTo   = t->end.isValid() ? t->end : t->date;
        m_definingElement = element;
        return true;
    }
    return false;
}
/*
        InitialDiagnosis,
        Therapy,
        BestSupportiveCare,
        FollowUp,
        Deceased,
        LossOfContact
*/

void EffectiveStateIterator::reinitialized()
{
    m_effectiveState = DiseaseState::UnknownState;
    m_stateValidTo   = QDate();
    m_definingElement = 0;
}

DiseaseState::State EffectiveStateIterator::effectiveState() const
{
    return m_effectiveState;
}

HistoryElement* EffectiveStateIterator::definingElement() const
{
    return m_definingElement;
}

// --------------------------------------------------------
/*
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
         * /
class TherapyGroup
{
public:
    TherapyGroup(const QString& s1, const QString& s2);
    TherapyGroup(const QStringList& s1, const QStringList& s2);

};*/

TherapyGroup::TherapyGroup()
{
}

TherapyGroup::TherapyGroup(const QList<Therapy*>& l)
    : QList<Therapy*>(l)
{
}

bool TherapyGroup::isContinuation(const Therapy* t) const
{
    if (isEmpty())
    {
        return false;
    }

    QSet<QString> therapies = substances();
    QSet<QString> newTherapies = QSet<QString>::fromList(t->elements.substances());

    // a subset? = deescalation
    if (therapies.contains(newTherapies))
    {
        return true;
    }

    QSet<QString> common = therapies.intersect(newTherapies);
    if (common.isEmpty())
    {
        return false;
    }

    return false;
}

QSet<QString> TherapyGroup::substances() const
{
    QSet<QString> therapies;
    foreach (const Therapy*t, *this)
    {
        foreach (const QString& s, t->elements.substances())
        {
            therapies += s;
        }
    }
    return therapies;
}

QDate TherapyGroup::beginDate() const
{
    QDate begin;
    foreach (const Therapy*t, *this)
    {
        if (begin.isNull() || t->begin() < begin)
        {
            begin = t->begin();
        }
    }
    return begin;
}

QDate TherapyGroup::endDate() const
{
    QDate end;
    foreach (const Therapy*t, *this)
    {
        if (end.isNull() || t->end > end)
        {
            end = t->end;
        }
    }
    return end;
}



NewTreatmentLineIterator::NewTreatmentLineIterator()
    :  m_seenProgression(false), m_newLineTherapy(0)
{
}

bool NewTreatmentLineIterator::isInterested(HistoryElement* element)
{
    return element->is<Therapy>() || element->is<Finding>();
}

bool NewTreatmentLineIterator::visit(HistoryElement* element)
{
    if (element->is<Therapy>())
    {
        Therapy* t = element->as<Therapy>();
        bool isNewLine = false, includeTherapy = false;
        int daysDistance = 0;
        if (!m_therapies.isEmpty())
        {
            daysDistance = m_therapies.last().endDate().daysTo(t->begin());
        }
        const int daysNewLineLimit = 90;
        if (t->type == Therapy::CTx)
        {
            bool isContinued = false;
            if (!m_therapies.isEmpty())
            {
                isContinued = m_therapies.last().isContinuation(t);
            }
            //qDebug() << "Therapie" << t->elements.substances() << t->begin() << t->end
              //       << "Fortführung" << isContinued << "PD" << m_seenProgression
                 //    << "Abstand" << daysDistance;

            if (m_seenProgression)
            {
                if (isContinued)
                {
                    debugOutput(t, "Kein Substanzwechsel trotz Progress");
                }
                isNewLine = true;
            }
            else
            {
                if (isContinued)
                {
                    if (daysDistance > daysNewLineLimit)
                    {
                        isNewLine = true;
                        debugOutput(t, "Keine Progression, Therapiewiederbeginn nach 90 Tagen");
                    }
                    else
                    {
                        includeTherapy = true;
                    }
                }
                else
                {
                    if (!m_therapies.isEmpty())
                    {
                        debugOutput(t, "Keine Progression, Substanzwechsel");
                    }
                    isNewLine = true;
                }
            }
        }
        else if (t->type == Therapy::RCTx || t->type == Therapy::RTx)
        {
            if (m_seenProgression || daysDistance > daysNewLineLimit)
            {
                isNewLine = true;
            }
            else
            {
                includeTherapy = true;
            }
        }
        // ignore other

        if (isNewLine)
        {
            TherapyGroup group;
            group << t;
            m_therapies << group;
            //qDebug() << "New Line" << t->elements.substances() << m_therapies.size()
                     //<< (m_therapies.isEmpty()? QString("leer") : QString::number(m_therapies.last().size()));
            m_newLineTherapy = t;
            m_seenProgression = false;
            return true;
        }
        else if (includeTherapy)
        {
            if (m_therapies.isEmpty())
            {
                qDebug() << "Wrong behavior: Cannot include therapy in empty list";
                m_therapies << TherapyGroup();
            }
            m_therapies.last() << t;
        }
    }
    else if (element->is<Finding>())
    {
        Finding* f = element->as<Finding>();
        if ((f->result == Finding::ProgressiveDisease || f->result == Finding::Recurrence)
                && !(f->additionalInfos & Finding::CentralNervous) )
        {
            m_seenProgression = true;
        }
    }
    return false;
}

Therapy* NewTreatmentLineIterator::currentTherapy() const
{
    return m_newLineTherapy;
}

QList<TherapyGroup> NewTreatmentLineIterator::therapies() const
{
    return m_therapies;
}

void NewTreatmentLineIterator::reinitialized()
{
    m_seenProgression = false;
    m_newLineTherapy  = 0;
    m_therapies.clear();
}

void NewTreatmentLineIterator::debugOutput(const Therapy* t, const QString& problem) const
{
    qDebug() << problem;
    if (m_therapies.isEmpty())
    {
        qDebug() << ": No previous therapy; new Therapy"
                 << t->begin() << t->end << t->elements.substances();
    }
    else
    {
        qDebug() << ": Previous therapy" << m_therapies.last().beginDate() << m_therapies.last().endDate()
                 << QStringList(m_therapies.last().substances().toList()).join(",")
                 << "; new Therapies" << t->begin() << t->end << t->elements.substances();
    }
}
