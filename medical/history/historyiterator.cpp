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
    : m_state(Invalid), m_proofreader(0)
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

void HistoryIterator::setProofreader(HistoryProofreader* pr)
{
    m_proofreader = pr;
}

void HistoryIterator::reportProblem(const HistoryElement* e, const QString& problem) const
{
    if (m_proofreader)
    {
        m_proofreader->problem(e, problem);
    }
    else
    {
        qDebug() << problem;
    }
}


// -------------------------------------------------------

/*
  // Code works, but is a bit simplistic
class CurrentStateIterator : public HistoryIterator
{
public:
    CurrentStateIterator();
    CurrentStateIterator(const Disease& disease);

    / ** Returns the last found DiseaseState of a history.
        Note: iterator finishes always atEnd()
        * /
    DiseaseState::State state() const;

    virtual bool isInterested(HistoryElement* element);
    virtual bool visit(HistoryElement* element);
protected:
    virtual void restarting();

    DiseaseState* lastState;
};

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

*/

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
    QDate begin = initialDiagnosis;
    switch (definition)
    {
    case FromInitialDiagnosis:
        break;
    case FromFirstTherapy:
        if (firstTherapy)
        {
            begin = firstTherapy->begin();
        }
        else
        {
            reportProblem(0, "OSIterator: no therapy recorded, using initial diagnosis");
        }
        break;
    }
    if (!begin.isValid())
    {
        reportProblem(0, QString("Invalid begin date")
                       + QString::number(definition)
                       + initialDiagnosis.toString());
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
        if (lastElement->is<Therapy>())
        {
            Therapy* t = lastElement->as<Therapy>();
            if (!t->end.isValid())
            {
                QDate lastDoc = m_history.lastDocumentation();
                if (lastDoc.isValid() && lastDoc > end)
                {
                    end = lastDoc;
                }
            }
        }
    }
    else
    {
        reportProblem(0, "Empty history, no OS");
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
      m_definingElement(0),
      m_therapyForValidTo(0)
{
    set(history);
}

bool EffectiveStateIterator::isInterested(HistoryElement* element)
{
    return true;
}

QDate EffectiveStateIterator::stateValidTo() const
{
    if (!m_therapyForValidTo)
    {
        return QDate();
    }
    // ensure a single-date therapy has a valid end date
    switch (m_therapyForValidTo->type)
    {
    case Therapy::CTx:
    case Therapy::RCTx:
    {
        if (m_therapyForValidTo->end.isValid())
        {
            return m_therapyForValidTo->end;
        }
        qDebug() << "Therapy without end from" << m_therapyForValidTo->date
                 << "last doc is" << m_history.lastDocumentation()
                 << "history end" << m_history.end();
        QDate lastDoc = m_history.lastDocumentation();
        if (lastDoc.isValid())
        {
            return lastDoc;
        }
        return m_history.end();
    }
    default:
        return m_therapyForValidTo->end.isValid() ? m_therapyForValidTo->end : m_therapyForValidTo->date;
    }
}

bool EffectiveStateIterator::visit(HistoryElement* element)
{
    // treat as invariable
    if (m_effectiveState == DiseaseState::Deceased)
    {
        return false;
    }

    m_therapyForValidTo = 0;
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
        Finding* f = element->as<Finding>();
        if (m_effectiveState < DiseaseState::InitialDiagnosis)
        {
            if (f->context == Finding::InitialDiagnosis)
            {
                m_effectiveState = DiseaseState::InitialDiagnosis;
                m_definingElement = element;
                return true;
            }
        }
        if (f->type == Finding::Death)
        {
            m_effectiveState  = DiseaseState::Deceased;
            m_definingElement = f;
            return true;
        }
    }
    else // if (element->is<Therapy>())
    {
        Therapy* t = element->as<Therapy>();
        m_effectiveState = DiseaseState::Therapy;
        m_definingElement = element;
        m_therapyForValidTo = t;
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
    m_effectiveState    = DiseaseState::UnknownState;
    m_therapyForValidTo = 0;
    m_definingElement   = 0;
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

CurrentStateIterator::CurrentStateIterator(const DiseaseHistory& history)
{
    set(history);
    start();
}

bool CurrentStateIterator::visit(HistoryElement* element)
{
    EffectiveStateIterator::visit(element);
    // always returning false makes it run through
    return false;
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

QDate TherapyGroup::effectiveEndDate() const
{
    QDate end = endDate();
    // return latest end
    if (end.isValid())
    {
        return end;
    }
    // return effective end (based on last documentation
    if (m_effectiveEndDate.isValid())
    {
        return m_effectiveEndDate;
    }
    // return latest begin
    foreach (const Therapy*t, *this)
    {
        if (end.isNull() || t->begin() > end)
        {
            end = t->begin();
        }
    }
    return end;
}

bool TherapyGroup::hasChemotherapy() const
{
    foreach (const Therapy*t, *this)
    {
        if (t->type == Therapy::CTx || t->type == Therapy::RCTx)
        {
            return true;
        }
    }
    return false;
}

NewTreatmentLineIterator::NewTreatmentLineIterator()
    :  m_seenProgression(false), m_isInTherapyBlock(false), m_newLineTherapy(0)
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
              //       << "Fortf�hrung" << isContinued << "PD" << m_seenProgression
                 //    << "Abstand" << daysDistance;

            if (m_isInTherapyBlock)
            {
                includeTherapy = true;
            }
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

        // evaluate manual definition of keeping therapies within a block/line
        if (t->additionalInfos & Therapy::BeginsTherapyBlock)
        {
            m_isInTherapyBlock = true;
        }
        else if (t->additionalInfos & Therapy::EndsTherapyBlock)
        {
            m_isInTherapyBlock = false;
        }

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
                reportProblem(t, "Wrong behavior: Cannot include therapy in empty list");
                m_therapies << TherapyGroup();
            }
            m_therapies.last() << t;
        }

        // adjust effective date each time
        if ((isNewLine || includeTherapy) && !m_therapies.isEmpty())
        {
            if (m_therapies.last().endDate().isNull()
                    && m_therapies.last().hasChemotherapy()
                    && m_history.lastDocumentation().isValid())
            {
                therapies().last().m_effectiveEndDate = m_history.lastDocumentation();
            }
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
    QString p = problem;
    if (m_therapies.isEmpty())
    {
        p += ": No previous therapy; new Therapy "
                + t->begin().toString() + " "
                + t->end.toString() + " "
                + t->elements.substances().join(",");
    }
    else
    {
        p += ": Previous therapy "
                + m_therapies.last().beginDate().toString() + " "
                + m_therapies.last().endDate().toString() + " "
                + QStringList(m_therapies.last().substances().toList()).join(",")
                + "; new Therapies "
                + t->begin().toString() + " "
                + t->end.toString() + " "
                + t->elements.substances().join(",");
    }
    reportProblem(t, p);
}