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

#ifndef DISEASE_H
#define DISEASE_H

// Qt includes

#include <QList>
#include <QDate>
#include <QSharedPointer>

// Local includes

#include "pathology.h"
#include "tnm.h"

class DiseaseHistory;

class HistoryEventInfo
{
public:
    QString type;
    QString info;
};

class HistoryEvent
{
public:
    QString eventClass;
    QDate   date;
    QString type;

    QList<HistoryEventInfo> infos;
};

class Disease
{
public:

    Disease();

    typedef QSharedPointer<Disease> Ptr;

public:

    QDate initialDiagnosis;
    TNM   initialTNM;
    QList<Pathology> pathologies;
    PropertyList     diseaseProperties;

    int   id;

    // Looks through pathologies and returns first found entity
    Pathology::Entity entity() const;

    bool hasPathology() const;
    Pathology& firstPathology();
    const Pathology& firstPathology() const;

    bool hasProfilePathology() const;
    Pathology& firstProfilePathology();
    const Pathology& firstProfilePathology() const;

    // context as defined by PathologyContextInfo
    bool hasPathology(int context) const;
    Pathology& firstPathology(int context);
    const Pathology& firstPathology(int context) const;

    bool hasPathology(const QString& id) const;
    Pathology& firstPathology(const QString& id);
    const Pathology& firstPathology(const QString& id) const;

    // Looks through properties of all pathology properties
    /**
      Returns the first Property from the pathologies with the
      given property key, or an invalid property if not found.
      */
    Property pathologyProperty(const QString& prop) const;
    /**
      Returns all Properties from the pathologies with the
      given property key.
      */
    PropertyList pathologyProperties(const QString& prop) const;
    /**
        Returns true if the given property is found.
        A null string acts as a wildcard.
      */
    bool hasPathologyProperty(const QString& prop,
                              const QString& value = QString(),
                              const QString& detail = QString()) const;

    /**
      Returns all properties from all pathologies
    */
    PropertyList allPathologyProperties() const;

    DiseaseHistory history() const;
    void setHistory(const DiseaseHistory& history);
};

#endif // DISEASE_H
