/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 25.02.2013
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

#ifndef XMLSTREAMUTILS_H
#define XMLSTREAMUTILS_H

#include <QFlags>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

/**
  Helper classes with convenience overrides.
  Optimized to work with the TEXT_INT_MAPPER from xmltextintmapper.h
  */

class XmlStreamWriter : public QXmlStreamWriter
{
public:
    XmlStreamWriter(QString* s) : QXmlStreamWriter(s) {}

    void writeAttribute(const QString &qualifiedName, const QString& value)
    {
        QXmlStreamWriter::writeAttribute(qualifiedName, value);
    }

    void writeAttribute(const QString &qualifiedName, int value)
    {
        QXmlStreamWriter::writeAttribute(qualifiedName, QString::number(value));
    }
    void writeAttribute(const QString &qualifiedName, const QDate &value)
    {
        QXmlStreamWriter::writeAttribute(qualifiedName, value.toString(Qt::ISODate));
    }

    void writeAttributeChecked(const QString &qualifiedName, const QString &value)
    {
        if (!value.isEmpty())
        {
            QXmlStreamWriter::writeAttribute(qualifiedName, value);
        }
    }
    void writeAttributeChecked(const QString &qualifiedName, int value)
    {
        if (value)
        {
            writeAttribute(qualifiedName, value);
        }
    }
    void writeAttributeChecked(const QString &qualifiedName, const QDate &value)
    {
        if (value.isValid())
        {
            writeAttribute(qualifiedName, value);
        }
    }
    template <typename Flag, class Mapper>
    void writeFlagAttribute(Flag flag, QFlags<Flag> flags)
    {
        if (flags & flag)
        {
            QXmlStreamWriter::writeAttribute(Mapper::toString(flag), "true");
        }
    }
    template <typename Flag, class Mapper>
    void writeFlagAttributes(QFlags<Flag> flags, const QList<Flag>& possibleFlags)
    {
        foreach (Flag flag, possibleFlags)
        {
            writeFlagAttribute<Flag, Mapper>(flag, flags);
        }
    }
};

class XmlStreamReader : public QXmlStreamReader
{
public:
    XmlStreamReader(const QString& s) : QXmlStreamReader(s) {}

    void readAttributeChecked(const QString &qualifiedName, QString& value)
    {
        value = attributes().value(qualifiedName).toString();
    }

    void readAttributeChecked(const QString &qualifiedName, int& value)
    {
        bool ok;
        int i = attributes().value(qualifiedName).toString().toInt(&ok);
        if (ok)
        {
            value = i;
        }
    }

    template <typename T, class Mapper>
    void readAttributeCheckedEnum(const QString &qualifiedName, T& value)
    {
        QStringRef s = attributes().value(qualifiedName);
        if (!s.isEmpty())
        {
            value = Mapper::toEnum(s);
        }
    }

    void readAttributeChecked(const QString &qualifiedName, QDate& value)
    {
        QDate d = QDate::fromString(attributes().value(qualifiedName).toString(), Qt::ISODate);
        if (d.isValid())
        {
            value = d;
        }
    }

    template <typename Flag, class Mapper>
    void readFlagAttribute(Flag flag, QFlags<Flag>& flags)
    {
        if (attributes().value(Mapper::toString(flag)) == "true")
        {
            flags |= flag;
        }
    }

    template <typename Flag, class Mapper>
    void readFlagAttributes(QFlags<Flag>& flags, const QList<Flag>& possibleFlags)
    {
        foreach (Flag flag, possibleFlags)
        {
            readFlagAttribute<Flag, Mapper>(flag, flags);
        }
    }

};

#endif // XMLSTREAMUTILS_H
