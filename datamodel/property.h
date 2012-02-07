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

#include <QString>

class Property
{
public:
    Property();
    Property(const QString& property, const QString& value, const QString& detail = QString());

    bool operator==(const Property& other);

    /// A property is valid if the "property" key is valid
    bool isValid() const;
    /// A property is empty if it is not valid, or if the value is empty
    bool isEmpty() const;

public:

    QString property;
    QString value;
    QString detail;
};

#endif // PROPERTY_H
