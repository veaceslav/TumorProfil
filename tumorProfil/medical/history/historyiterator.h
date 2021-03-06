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

#ifndef HISTORYITERATOR_H
#define HISTORYITERATOR_H

#include "diseasehistory.h"
#include "disease.h"

class HistoryProofreader
{
public:
    virtual void problem(const HistoryElement* element, const QString& problem) = 0;
    virtual void reset() {}
};

class HistoryIterator
{
public:
    HistoryIterator();

    enum State
    {
        Invalid,
        Initialized,
        Match,
        AtEnd
    };

    /**
      Initialize beginning with
       a) the given HistoryElement (default: first element) of the given history
       b) the current element of the other iterator
       c) the given iterator
      Start from
       d) the current element
       e) the element following the current element.
       f) a composite which is the current element at the beginning and the next element afterwards
      Returns currentState().
      */
    void set(const DiseaseHistory& history, HistoryElement* e = 0);
    void set(const HistoryIterator& other);
    void set(const DiseaseHistory& history, HistoryElementList::const_iterator it);
    State start();
    State startNext();
    State next();

    /// Effectively: while (!atEnd()) next();
    State iterateToEnd();

    State currentState() const;
    HistoryElement* currentElement() const;

    bool hasMatch() const { return currentState() == Match; }
    bool atEnd() const { return currentState() == AtEnd; }
    bool isValid() const { return currentState() != Invalid; }

    /**
      Reimplement: Return true if the iterator is interested in the given element.
      It will then be visited. Otherwise it will be skipped.
      */
    virtual bool isInterested(HistoryElement* element) = 0;
    /**
      Called with any element that this iterator is interested in.
      If true is returned, the State "Match" is entered and iteration ends.
      If false is returend, the iteration continues.
      */
    virtual bool visit(HistoryElement* element) = 0;

    void setProofreader(HistoryProofreader* pr);

protected:

    /** Called when a new history is set on the iterator.
        Typically, you may want to reset some state here.
        restarting() will always be called as well.
        */
    virtual void reinitialized();
    /** Called from start() before the iteration begins.
        Typically, you may want to reset some state here.
        */
    virtual void restarting();

    /** Worker method:
        Starting with current, calls visit on any element that the iterator isInterested() in.
        */
    void iterate();

    // reports via proofreader if there is a proofreader, else through qDebug
    void reportProblem(const HistoryElement* e, const QString& problem) const;

    DiseaseHistory m_history;
    HistoryElementList::const_iterator m_current;
    State m_state;
    HistoryProofreader* m_proofreader;
};

class OSIterator : public HistoryIterator
{
public:
    /**
      Constructs and start the OS iterator on the given disease's history.
      The iterator always finishes atEnd();
      */
    OSIterator(const Disease& disease);

    enum Definition
    {
        FromInitialDiagnosis,
        FromFirstTherapy
    };

    int days(Definition definition = FromInitialDiagnosis) const;
    int days(HistoryElement* from) const;
    bool endpointReached() const;
    QDate beginDate(Definition definition) const;
    QDate endDate() const;
    float months(Definition definition = FromInitialDiagnosis) const;

    virtual bool isInterested(HistoryElement* element);
    virtual bool visit(HistoryElement* element);

protected:

    int days(const QDate& begin) const;
    virtual void restarting();

    QDate initialDiagnosis;
    Therapy* firstTherapy;
    HistoryElement* lastElement;
    HistoryElement* endpointElement;
};

class ProgressionIterator : public HistoryIterator
{
public:
    enum ProgressionCategory
    {
        AnyProgression,
        ExcludeCNS,
        OnlyRecurrence
    };
    ProgressionIterator(ProgressionCategory category = AnyProgression);

    Finding* progression() const;

protected:
    virtual bool isInterested(HistoryElement* element);
    virtual bool visit(HistoryElement* element);
    ProgressionCategory category;
};

class EffectiveStateIterator : public HistoryIterator
{
public:

    EffectiveStateIterator();

    virtual bool isInterested(HistoryElement* element);
    virtual bool visit(HistoryElement* element);

    DiseaseState::State effectiveState() const;
    // If there is an effectiveState, it may have a limited duration
    // after which an element should indicate a new state.
    QDate stateValidTo() const;
    // The history element which defined the current effective state
    HistoryElement* definingElement() const;

protected:
    virtual void reinitialized();

    DiseaseState::State m_effectiveState;
    HistoryElement *m_definingElement;
    Therapy*        m_therapyForValidTo;
};

class CurrentStateIterator : public EffectiveStateIterator
{
public:
    /** Runs through an finishes atEnd() */
    CurrentStateIterator(const DiseaseHistory& history);
    QDate effectiveHistoryEnd() const;

    virtual bool visit(HistoryElement* element);
};


class TherapyGroup : public QList<Therapy*>
{
public:
    TherapyGroup();
    TherapyGroup(const QList<Therapy*> &);
    bool isContinuation(const Therapy* t) const;

    // First and last by begin/end date (not equivalent to QList::first()/last())
    Therapy* firstTherapy() const;
    Therapy* lastTherapy() const;

    QDate beginDate() const;
    // can be null
    QDate endDate() const;
    // cannot be null
    QDate effectiveEndDate() const;
    QSet<QString> substances() const;
    bool hasChemotherapy() const;
    bool hasSubstance(const QString& substance) const;
    bool hasSurgery() const;
    bool hasRadiotherapy() const;

protected:

    friend class NewTreatmentLineIterator;
    QDate m_effectiveEndDate;
};

class NewTreatmentLineIterator : public HistoryIterator
{
public:

    NewTreatmentLineIterator();

    virtual bool isInterested(HistoryElement* element);
    virtual bool visit(HistoryElement* element);

    /// Returns the therapy which constituted a new line.
    /// Note that the full group may not be available,
    /// in case following therapies will be counted as part of the same therapy.
    Therapy* currentTherapy() const;
    /// Returns all therapy groups encountered so far.
    /// If at end, will have the complete list.
    QList<TherapyGroup> therapies() const;

protected:
    virtual void reinitialized();

    bool m_seenProgression;
    bool m_isInTherapyBlock;

    void debugOutput(const Therapy* t, const QString& problem) const;
    void adjustEffectiveEndDate(const QDate& endDate);

    QList<TherapyGroup> m_therapies;
    Therapy* m_newLineTherapy;
};

#endif // HISTORYITERATOR_H
