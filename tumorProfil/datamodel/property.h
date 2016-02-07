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

#ifndef PROPERTY_H
#define PROPERTY_H

// Qt includes

#include <QList>
#include <QMetaType>
#include <QString>

class Property
{
public:
    Property();
    Property(const QString& property, const QString& value, const QString& detail = QString());

    bool operator==(const Property& other) const;
    bool operator<(const Property& other) const;

    /// A property is valid if the "property" key is valid
    bool isValid() const;
    bool isNull() const { return !isValid(); }
    /// A property is empty if it is not valid, or if the value is empty
    bool isEmpty() const;

public:

    QString property;
    QString value;
    QString detail;
};

class PropertyList : public QList<Property>
{
public:

    PropertyList();
    PropertyList(const QList<Property>& list);

    /**
      Returns the first Property from this list with the
      given property key, or an invalid property if not found.
      */
    Property property(const QString& prop) const;
    /**
      Returns all Properties from this list with the
      given property key. Returns the empty list if none found.
      */
    PropertyList properties(const QString& prop) const;
    /**
        Returns true if the given property is found.
        A null string acts as a wildcard.
      */
    bool hasProperty(const QString& prop,
                     const QString& value = QString(),
                     const QString& detail = QString()) const;

    /**
      Sets the given property. Removes any previous entry.
      */
    void setProperty(const QString& prop, const QString& value, const QString& detail = QString());

    /**
      Adds the given property. Allows multiple entries.
      */
    void addProperty(const QString& prop, const QString& value, const QString& detail = QString());

    /**
      Removes all matching properties.
      */
    void removeProperty(const QString& prop, const QString& value = QString(), const QString& detail = QString());

    /**
      Merge the given list into this list. The given list has priority,
      if properties are present in both lists, the property from the given list replaces the property of this list.
      */
    void merge(const PropertyList& other);
};

Q_DECLARE_METATYPE(Property)

#endif // PROPERTY_H
