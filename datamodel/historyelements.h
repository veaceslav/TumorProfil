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
};

template <class E>
class GenericElementList : public QList<E*>
{
public:
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
                do { ++it; } while (it != end && dynamic_cast<const T*>(*it));
                return *this;
            }

        protected:
            typename GenericElementList::const_iterator it, end;
        };

        const_iterator begin() const { return const_iterator(list.begin(), list.end()); }
        const_iterator end() const   { return const_iterator(list.end(), list.end()); }

    protected:
        GenericElementList list;
    };

    template <class T>
    FilteredList<T> filtered() const { return FilteredList<T>(*this); }

    template <class T>
    const T* first() const { foreach (const T* t, filtered<T>()) { return t; }; return 0; }
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
    /// repeat q days
    int repeat;
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
    QStringList substances() const;
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

    QDate begin() const { return date; }
    QDate end;

    Type type;
    QString description;
    TherapyElementList elements;
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

    Type    type;
    Result  result;
    Context context;
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
        LossOfContact
    };

    QDate dateOfUpdate;
    State state;
};

#endif // HISTORYELEMENTS_H
