/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 2012-01-22
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

#ifndef TNM_H
#define TNM_H

// Qt includes

#include <QString>
#include <QStringList>

class TNMData
{
public:

    enum ClinicalOrPathological
    {
        c = 'c',
        p = 'p'
    };

    TNMData(ClinicalOrPathological type = c);

    ClinicalOrPathological type;
    QByteArray             flags; //yarm
    QString                T;
    QString                N;
    QStringList            attributesN;
    QString                M;
    QStringList            attributesM;
    char                   L;
    char                   V;
    char                   G;
    char                   R;
};

class TNM
{
public:

    enum StageConflictBehavior
    {
        OnConflictChooseLowerStage,
        OnConflictChooseHigherStage,
        OnConflictChooseX
    };

public:
    TNM();

    /**
      Parses the given string and fills the TNM data.
      The string may contain both c and p tnm elements.
      */
    TNM(const QString& tnm);
    TNM(const TNMData& tnm);

    /**
      Will parse the string and merge the contained information into the current data.
      */
    void addTNM(const QString& tnm,
                StageConflictBehavior behavior = OnConflictChooseLowerStage);
    /**
      Replaces the current data with the given string.
      Note: Will replace both cTNM and pTNM.
      */
    void setTNM(const QString& tnm);
    /**
      Replaces the current data - either cTNM or pTNM.
      */
    void setTNM(const TNMData& tnm);

    /**
      Checks both cTNM and pTNM and returns the more valid value
      */
    QString T() const;
    QString N() const;
    QString M() const;

    QString cTNM() const;
    QString pTNM() const;

    TNMData& data(TNMData::ClinicalOrPathological type = TNMData::c)
        { return (type == TNMData::c) ? m_cTNM : m_pTNM; }
    const TNMData& data(TNMData::ClinicalOrPathological type = TNMData::c) const
        { return (type == TNMData::c) ? m_cTNM : m_pTNM; }

    /**
      Merges two TNMData, which shall be of same type.
      In the result, if there is a non-x in either data, it will be taken;
      In the case of a conflict, the StageConflictBehavior decides.
      Attributes of N and M as well as flags are merged.
      */
    static TNMData merge(const TNMData& a, const TNMData&b,
                         StageConflictBehavior behavior = OnConflictChooseLowerStage);

public:

    TNMData m_cTNM;
    TNMData m_pTNM;
};

#endif // TNM_H
