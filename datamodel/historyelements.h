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

#ifndef HISTORYELEMENTS_H
#define HISTORYELEMENTS_H

// Qt includes

#include <QDate>
#include <QFlags>
#include <QMetaType>
#include <QString>
#include <QStringList>

class HistoryElement
{
public:
    HistoryElement();
    virtual ~HistoryElement();

    QDate date;

    template <class T>
    bool is() const { return dynamic_cast<const T*>(this); }
    template <class T>
    const T* as() const { return static_cast<const T*>(this); }
    template <class T>
    T* as() { return static_cast<T*>(this); }

    HistoryElement* parent() const;
    void setParent(HistoryElement* parent);

protected:

    HistoryElement* m_parent;
};

template <class E>
class GenericElementList : public QList<E*>
{
public:
    GenericElementList(HistoryElement* parent = 0) : m_parent(parent) {}
    /**
      Allows to have automatic filtering like
      foreach (const Chemotherapy* ctx, list.filtered<Chemotherapy>())
      where list contains HistoryElements of different types.
      */
    template <class T>
    class FilteredList
    {
    public:
        FilteredList(const GenericElementList& list) : list(list) {}
        class const_iterator
        {
        public:
            const_iterator(typename GenericElementList::const_iterator it,
                           typename GenericElementList::const_iterator end) : it(it), end(end) {}
            const T* operator*() { return static_cast<T*>(*it); }
            bool operator!=( const const_iterator& o) const { return it != o.it; }
            const_iterator& operator++()
            {
                ++it;
                return advance();
            }

            const_iterator& advance()
            {
                while (it != end && !dynamic_cast<const T*>(*it)) { ++it; }
                return *this;
            }

        protected:
            typename GenericElementList::const_iterator it, end;
        };

        const_iterator begin() const { const_iterator b(list.begin(), list.end()); b.advance(); return b; }
        const_iterator end() const   { return const_iterator(list.end(), list.end()); }

    protected:
        GenericElementList list;
    };

    template <class T>
    FilteredList<T> filtered() const { return FilteredList<T>(*this); }

    template <class T>
    const T* first() const { foreach (const T* t, filtered<T>()) { return t; } return 0; }

    HistoryElement* parent() const { return m_parent; };

    template <class T>
    typename QList<E*>::const_iterator find(const T& functor, typename QList<E*>::const_iterator it) const
    {
        if (it == QList<E*>::end())
        {
            return it;
        }
        typename QList<E*>::const_iterator found = it;
        for (++it; it != QList<E*>::end(); ++it)
        {
            if (functor(it, found))
            {
                found = it;
            }
        }
        return found;
    }

    template <class Functor>
    E* findPointer(const Functor& functor, typename QList<E*>::const_iterator it) const
    {
        typename QList<E*>::const_iterator result = find(functor, it);
        if (result == QList<E*>::end())
            return 0;
        return *result;
    }

    template <class Functor> struct dateFunctor
    {
        bool operator() (const typename QList<E*>::const_iterator& x, const typename QList<E*>::const_iterator& y) const
        {return Functor()((*x)->date, (*y)->date);}
    };

    /*template <class Functor functor>
    struct dateFunctor2
    {
        bool operator() (const const_iterator& x, const const_iterator& y) const
        {return functor(x->date, y->date);}
    };*/


    // sorts by date
    E* latestByDate() const
    {
        return findPointer(dateFunctor<qGreater<QDate> >(), QList<E*>::begin());
    }

    HistoryElement* firstByDate() const
    {
        return findPointer(dateFunctor<qLess<QDate> >(), QList<E*>::begin());
    }

protected:

    HistoryElement* m_parent;
};

class HistoryElementList : public GenericElementList<HistoryElement>
{
};

// ---------------------------


class TherapyElement : public HistoryElement
{
};

class Chemotherapy : public TherapyElement
{
public:
    Chemotherapy();

    /**
      Chemotherapy substance:
      Cisplatin
      Carboplatin
      Gemcitabine
      Vinorelbine
      Pemetrexed
      5-FU
      Irinotecan
      Oxaliplatin
      Mitomycin
      Ifosfamide
      Paclitaxel
      Docetaxel
      */
    QString substance;
    /// mg/m2
    int dose;
    /// mg abs.
    int absdose;
    /// schedule (days of administration, repeat)
    QString schedule;

    static QStringList substances();
};

class Radiotherapy : public TherapyElement
{
public:
    Radiotherapy();

    QString location;
    /// Gy (cumulative)
    int dose;
};

class Toxicity : public TherapyElement
{
public:
    Toxicity();

    QString description;
    /// CTCAE 4.0 grade (optional), 0 = not defined
    int grade;
};

class TherapyElementList : public GenericElementList<TherapyElement>
{
public:
    TherapyElementList(HistoryElement* parent);
    QStringList substances() const;
    bool hasSubstance(const QString& substance) const;
    /// Convenience method: adjust the element's parent.
    /// You must do that yourself if you use append, prepend, or insert.
    TherapyElementList& operator<<(TherapyElement* elem);
};

class Therapy : public HistoryElement
{
public:
    Therapy();
    ~Therapy();

    enum Type
    {
        CTx,
        RTx,
        RCTx,
        Surgery,
        Intervention
    };

    enum AdditionalInfo
    {
        NoAdditionalInfo    = 0,
        BeginsTherapyBlock  = 1 << 0,
        EndsTherapyBlock    = 1 << 1
    };
    Q_DECLARE_FLAGS(AdditionalInfos, AdditionalInfo)

    QDate begin() const { return date; }
    QDate end;

    Type type;
    QString description;
    TherapyElementList elements;
    AdditionalInfos additionalInfos;

    static QString uiLabel(Type type);
};

// ---------------------------

class Finding : public HistoryElement
{
public:

    Finding();

    enum Type
    {
        UndefinedType,
        Clinical,
        Histopathological,
        CT,
        MRI,
        XRay,
        Sono,
        PETCT,
        Scintigraphy,
        Death
    };

    enum Result
    {
        UndefinedResult,
        StableDisease,
        ProgressiveDisease,
        MinorResponse,
        PartialResponse,
        CompleteResponse,
        NoEvidenceOfDisease,
        InitialFindingResult,
        Recurrence,
        ResultNotApplicable,

        SD = StableDisease,
        PD = ProgressiveDisease,
        MR = MinorResponse,
        PR = PartialResponse,
        CR = CompleteResponse,
        NED = NoEvidenceOfDisease
    };

    enum Context
    {
        UndefinedContext,
        Antecedent,
        InitialDiagnosis,
        ResponseEvaluation,
        FollowUp
    };

    enum AdditionalInfo
    {
        NoAdditionalInfo = 0,
        LocalRecurrence  = 1 << 0,
        Metastasis       = 1 << 1,
        CentralNervous   = 1 << 2
    };
    Q_DECLARE_FLAGS(AdditionalInfos, AdditionalInfo)

    Type    type;
    Result  result;
    Context context;
    AdditionalInfos additionalInfos;
    QString description;
};


// ---------------------------

class DiseaseState : public HistoryElement
{
public:
    DiseaseState();

    enum State
    {
        UnknownState,
        InitialDiagnosis,
        Therapy,
        BestSupportiveCare,
        FollowUp,
        Deceased,
        LossOfContact,
        WatchAndWait
    };

    QDate dateOfUpdate() const { return date; }
    State state;
};

Q_DECLARE_METATYPE(HistoryElement*)

#endif // HISTORYELEMENTS_H
