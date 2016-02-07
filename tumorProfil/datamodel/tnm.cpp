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

#include "tnm.h"

// Qt includes

#include <QDebug>
#include <QRegExp>

TNMData::TNMData(ClinicalOrPathological type)
    : type(type)
{
    T = 'x';
    N = 'x';
    M = 'x';
    L = 'x';
    V = 'x';
    G = 'x';
    R = 'x';
}

bool TNMData::isEmpty() const
{
    return
            T == "x" &&
            N == "x" &&
            M == "x" &&
            L == 'x' &&
            V == 'x' &&
            G == 'x' &&
            R == 'x';

}

class TNMParser
{
public:
    TNMParser(const QString& tnmString)
        : cTNM(TNMData::c), pTNM(TNMData::p)
    {
        QRegExp regexpT("([yarm]*)\\s*"
                        "(c|p)?T([\\dabcisx]{1,2})\\s*");
        QRegExp regexpN("(c|p)?N([\\dabcx]{1,2})\\s*"
                        "((?:\\((?:sn|mi|i\\-|i\\+|mol\\-|mol\\+)\\))*)\\s*");
        QRegExp regexpM("(c|p)?M([\\dabcx]{1,2})\\s*"
                        "(?:\\(((?:PUL|OSS|HEP|BRA|LYM|MAR|PLE|PER|ADR|SKI|OTH|\\s+|\\,)+)\\))?\\s*");
        QRegExp regexpLGVR("([LGVR])(\\d)");

        int index;
        bool hasT = false, hasN = false, hasM = false;
        QString s = tnmString;
        while ((index = regexpT.indexIn(s)) != -1)
        {
            TNMData& tnm = relevantData(regexpT.cap(2));
            tnm.flags = regexpT.cap(1).toLower().toLatin1();
            tnm.T = regexpT.cap(3).toLower();
            s.remove(index, regexpT.matchedLength());
            hasT = true;
        }
        while ((index = regexpN.indexIn(s)) != -1)
        {
            TNMData& tnm = relevantData(regexpN.cap(1));
            tnm.N = regexpN.cap(2).toLower();
            if (!regexpN.cap(3).isEmpty())
                tnm.attributesN << regexpN.cap(3).toLower(); //TODO: split ()
            s.remove(index, regexpN.matchedLength());
            hasN = true;
        }
        while ((index = regexpM.indexIn(s)) != -1)
        {
            TNMData& tnm = relevantData(regexpM.cap(1));
            tnm.M = regexpM.cap(2).toLower();
            tnm.attributesM = regexpM.cap(3).split(QRegExp("[\\s\\;\\,]+"), QString::SkipEmptyParts);
            s.remove(index, regexpM.matchedLength());
            hasM = true;
        }
        // some assume M0 if M is not given after T and N
        if (hasT && hasN && !hasM)
        {
            cTNM.M = QString();
            pTNM.M = QString();
        }
        while ((index = regexpLGVR.indexIn(s)) != -1)
        {
            char value = regexpLGVR.cap(2).toLower().toLatin1().at(0);
            switch (regexpLGVR.cap(1).toUpper().toLatin1().at(0))
            {
            case 'L':
                pTNM.L = value;
                break;
            case 'G':
                pTNM.G = value;
                break;
            case 'V':
                pTNM.V = value;
                break;
            case 'R':
                pTNM.R = value;
                break;
            }
            s.remove(index, regexpLGVR.matchedLength());
        }
    }

    TNMData cTNM;
    TNMData pTNM;

    TNMData& relevantData(const QString& cOrP)
    {
        if (cOrP == "p" || cOrP == "P")
        {
            return pTNM;
        }
        else // if (cOrP.isEmpty() || cOrP == 'c' || cOrP == 'C')
        {
            return cTNM;
        }
    }
};

class TNMBuilder
{
public:
    TNMBuilder(const TNMData& data)
        : tnm(data)
    {
    }

    enum FlagMode
    {
        OmitPC, // T1 N1 M1
        OnePC,  // cT1 N1 M1
        ThreePC // cT1 cN1 cM1
    };

    QString toString(FlagMode flagMode = ThreePC) const
    {
        QString s;
        s += tnm.flags;
        if (flagMode != OmitPC)
            s += (char)tnm.type;
        s += "T" + tnm.T + " ";
        if (flagMode == ThreePC)
            s += (char)tnm.type;
        s += "N" + tnm.N + " ";
        if (!tnm.attributesN.isEmpty())
            s += tnm.attributesN.join(QString()) += " ";
        if (flagMode == ThreePC)
            s += (char)tnm.type;
        s += "M" + tnm.M + " ";
        if (!tnm.attributesM.isEmpty())
            s += "(" + tnm.attributesM.join(" ") + ")" + " ";
        return s.trimmed();
    }

    TNMData tnm;
};

TNM::TNM()
{
}

TNM::TNM(const TNMData &tnm)
{
    setTNM(tnm);
}

TNM::TNM(const QString& tnm)
{
    setTNM(tnm);
}

bool TNM::isEmpty() const
{
    return m_pTNM.isEmpty() && m_cTNM.isEmpty();
}

void TNM::addTNM(const QString& tnm, StageConflictBehavior behavior)
{
    TNMParser parser(tnm);
    m_pTNM = merge(m_pTNM, parser.pTNM, behavior);
    m_cTNM = merge(m_cTNM, parser.cTNM, behavior);
}

void TNM::setTNM(const QString& tnm)
{
    m_TNMString = tnm;
    TNMParser parser(tnm);
    m_cTNM = parser.cTNM;
    m_pTNM = parser.pTNM;
}

void TNM::setTNM(const TNMData& tnm)
{
    data(tnm.type) = tnm;
}

QString TNM::cTNM() const
{
    TNMBuilder builder(m_cTNM);
    return builder.toString();
}

QString TNM::pTNM() const
{
    TNMBuilder builder(m_pTNM);
    return builder.toString();
}

static QString validStage(const QString& c, const QString& p)
{
    if (c == "x")
    {
        return p;
    }
    if (p == "x")
    {
        return c;
    }
    if (p.isEmpty())
    {
        return c;
    }
    return p;
}

QString TNM::T() const
{
    return validStage(m_cTNM.T, m_pTNM.T);
}

QString TNM::N() const
{
    return validStage(m_cTNM.N, m_pTNM.N);
}

QString TNM::M() const
{
    return validStage(m_cTNM.M, m_pTNM.M);
}

QVariant TNM::Tnumber() const
{
    QString t = T();
    t.remove(QRegExp("\\D"));
    if (t.isEmpty() || t == "x")
    {
        return QVariant();
    }
    return t.toInt();
}

QVariant TNM::Nnumber() const
{
    QString n = N();
    n.remove(QRegExp("\\D"));
    if (n.isEmpty() || n == "x")
    {
        return QVariant();
    }
    return n.toInt();
}

TNM::MStatus TNM::mstatus(MissingMInterpretation mmi) const
{
    QString m = M();
    if (m.isEmpty())
    {
        if (mmi == AssumeM0IfMissing)
        {
            return M0;
        }
        else
        {
            return Mx;
        }
    }
    if (m.contains("1"))
    {
        return M1;
    }
    else if (m.contains("0"))
    {
        return M0;
    }
    else if (m.contains("x", Qt::CaseInsensitive))
    {
        return Mx;
    }
    else
    {
        qDebug() << "failed to parse M value" << m << "assuming MX";
        return Mx;
    }
}

template <typename String>
static String mergeStage(const String& a, const String& b, TNM::StageConflictBehavior behavior)
{
    if (a == b)
    {
        return a;
    }
    if (a == String('x'))
    {
        return b;
    }
    else if (b == String('x'))
    {
        return a;
    }
    switch (behavior)
    {
    default:
    case TNM::OnConflictChooseLowerStage:
        return (QString::localeAwareCompare(QString(a),QString(b))<0) ? a : b;
    case TNM::OnConflictChooseHigherStage:
        return (QString::localeAwareCompare(QString(a),QString(b))>0) ? a : b;
    case TNM::OnConflictChooseX:
        return String('x');
    }
}

template <class List>
List mergeNoDuplicates(const List& a, const List& b)
{
    List result;
    result = a;
    for (int i=0; i<b.size(); i++)
    {
        if (!result.contains(b.at(i)))
        {
            result += b.at(i);
        }
    }
    return result;
}

TNMData TNM::merge(const TNMData& a, const TNMData&b, StageConflictBehavior behavior)
{
    TNMData result;
    result.type = a.type;
    result.flags = mergeNoDuplicates(a.flags, b.flags);
    result.T = mergeStage(a.T, b.T, behavior);
    result.N = mergeStage(a.N, b.N, behavior);
    result.M = mergeStage(a.M, b.M, behavior);
    result.attributesN = mergeNoDuplicates(a.attributesN, b.attributesN);
    result.attributesM = mergeNoDuplicates(a.attributesM, b.attributesM);
    result.L = mergeStage(a.L, b.L, behavior);
    result.G = mergeStage(a.G, b.G, behavior);
    result.R = mergeStage(a.R, b.R, behavior);
    result.V = mergeStage(a.V, b.V, behavior);
    return result;
}

QString TNM::toText() const
{
    if (!m_TNMString.isNull())
    {
        return m_TNMString;
    }
    return cTNM() + " " + pTNM();
}
