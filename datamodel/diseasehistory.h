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

#ifndef DISEASEHISTORY_H
#define DISEASEHISTORY_H

#include <QSharedDataPointer>

#include "historyelements.h"
#include "property.h"

class DiseaseHistory
{
public:
    DiseaseHistory();
    DiseaseHistory(const DiseaseHistory& other);
    ~DiseaseHistory();

    DiseaseHistory& operator=(const DiseaseHistory& other);
    bool operator==(const DiseaseHistory& other) const;
    bool operator!=(const DiseaseHistory& other) const { return !operator==(other); }

    bool isEmpty() const;
    int  size() const;
    QDate begin() const;
    QDate end() const;
    DiseaseState state() const;

    HistoryElementList& entries();
    const HistoryElementList& entries() const;

    HistoryElement* operator[](int i);
    const HistoryElement* operator[](int i) const;

    PropertyList& properties();
    const PropertyList& properties() const;
    void setLastDocumentation(const QDate& date);
    QDate lastDocumentation() const;

    void sort();

    static DiseaseHistory fromXml(const QString& xml);
    QString toXml() const;

    /// Removes an element from this history. Supports removal of child entries.
    /// Note: Does not delete the object.
    void remove(HistoryElement* e);

    DiseaseHistory& operator<<(HistoryElement*);

    QDate latestDate() const;

    class Private;
    static void test();

private:

    QSharedDataPointer<Private> d;
};

#endif // DISEASEHISTORY_H
