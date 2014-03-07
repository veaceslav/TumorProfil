/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 21.02.2012
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

#ifndef PATIENTPROPERTYFILTERMODEL_H
#define PATIENTPROPERTYFILTERMODEL_H

// Qt includes

#include <QDate>
#include <QSortFilterProxyModel>
#include <QMap>

// Local includes

#include "pathology.h"
#include "patient.h"

class PatientPropertyFilterSettings
{
public:

    PatientPropertyFilterSettings() : dateAppliesCombinedWithContext(true) {}

    QList<Pathology::Entity> entities;
    /**
      The key is the property id.
      The value can be boolean: Filters if the pathological test is pos. or neg.
      or a string: Filters if the value is identical.
      */
    QMap<QString, QVariant>   pathologyProperties;
    QMap<QString, QVariant>   pathologyPropertiesAnd;

    QMap<QString, bool> pathologyContexts;
    QMap<QString, bool> trialParticipation;

    QDate resultDateBegin;
    QDate resultDateEnd;
    bool  dateAppliesCombinedWithContext;

    bool matchesEntities(Patient::Ptr p) const;
    bool matchesPathologyProperties(Patient::Ptr p) const;
    bool matchesPathologyPropertiesAnd(Patient::Ptr p) const;
    bool matchesPathologyContexts(Patient::Ptr p) const;
    bool matchesTrialParticipation(Patient::Ptr p) const;
    bool matchesDates(Patient::Ptr p) const;

protected:

    enum Logic
    {
        And,
        Or
    };

    bool matchesPathologyProperties(Patient::Ptr p, const QMap<QString, QVariant>& pathProps,
                                    Logic logic) const;
};

class PatientPropertyFilterModel : public QSortFilterProxyModel
{
public:

    PatientPropertyFilterModel(QObject* parent = 0);
    ~PatientPropertyFilterModel();

    PatientPropertyFilterSettings filterSettings() const;
    void setFilterSettings(const PatientPropertyFilterSettings& settings);

    void filterByEntity(Pathology::Entity entity);
    void filterByEntity(const QList<Pathology::Entity> entities);
    void filterByPathologyProperty(const QString& property, const QVariant& value);
    void filterByPathologyProperty(const QMap<QString, QVariant>& filter);
    void filterByPathologyContext(const QString& property, bool value = true);
    void filterByTrialParticipation(const QString& property, bool value = true);

protected:

    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;

private:

    class PatientPropertyFilterModelPriv;
    PatientPropertyFilterModelPriv* const d;
};

#endif // PATIENTPROPERTYFILTERMODEL_H
