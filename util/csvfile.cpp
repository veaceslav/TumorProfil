/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 04.04.2012
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

#include "csvfile.h"


#include <QDate>
#include <QDebug>

CSVFile::CSVFile(const QChar& delimiter)
    : m_delimiter(delimiter)
{
}

bool CSVFile::read(const QString& filePath)
{
    m_file.setFileName(filePath);
    if (!m_file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << m_file.errorString();
        return false;
    }
    m_stream.setDevice(&m_file);
    return true;
}

bool CSVFile::openForWriting(const QString& filePath)
{
    m_file.setFileName(filePath);
    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << m_file.errorString();
        return false;
    }
    m_stream.setDevice(&m_file);
    return true;
}

void CSVFile::finishWriting()
{
    m_stream.flush();
    m_file.close();
    m_stream.setDevice(0);
}

void CSVFile::writeToString(QString *string)
{
    m_stream.setString(string);
}

bool CSVFile::atEnd() const
{
    return m_stream.atEnd();
}

CSVFile& CSVFile::operator<<(const QVariant& record)
{
    m_buffer << record;
    return *this;
}

void CSVFile::newLine()
{
    writeNextLine(m_buffer);
    m_buffer.clear();
}

void CSVFile::writeNextLine(const QList<QVariant>& records)
{
    //qDebug() << "Writing line with" << records.size();
    if (records.isEmpty())
    {
        return;
    }

    bool first = true;
    foreach (const QVariant& v, records)
    {
        if (!first)
            m_stream << m_delimiter;
        first = false;

        switch (v.type())
        {
        case QVariant::Date:
            m_stream << v.toDate().toString("dd.MM.yyyy");
            break;
        case QVariant::Bool:
            m_stream << v.toInt();
            break;
        case QVariant::String:
        default:
        {
            QString s = v.toString();
            if (s.contains(m_delimiter))
            {
                m_stream << '"' << v.toString() << '"';
            }
            else
            {
                m_stream << v.toString();
            }
            break;
        }
        }
    }

    m_stream << '\n';
}

QList<QVariant> CSVFile::parseNextLine()
{
    QString line = m_stream.readLine();
    QList<QVariant> records;

    if (line.isNull())
    {
        return records;
    }

    // code from http://www.zedwood.com/article/112/cpp-csv-parser
    int linepos = 0;
    bool inquotes = false;
    bool hadquotes = false;
    QChar c;
    const int linemax = line.length();
    QString curstring;

    while (linepos < linemax)
    {

        c = line.at(linepos);

        if (!inquotes && curstring.length()==0 && c=='"')
        {
            //beginquotechar
            inquotes = true;
            hadquotes = true;
        }
        else if (inquotes && c == '"')
        {
            //quotechar
            if ( (linepos+1 <linemax) && (line[linepos+1]=='"') )
            {
                //encountered 2 double quotes in a row (resolves to 1 double quote)
                curstring.push_back(c);
                linepos++;
            }
            else
            {
                //endquotechar
                inquotes=false;
            }
        }
        else if (!inquotes && c == m_delimiter)
        {
            //end of field
            records << toVariant(curstring, hadquotes);
            curstring.clear();
            hadquotes = false;
        }
        else if (!inquotes && (c=='\r' || c=='\n') )
        {
            records << toVariant(curstring, hadquotes);
            return records;
        }
        else
        {
            curstring.append(c);
        }
        linepos++;
    }
    records << toVariant(curstring, hadquotes);

    return records;
}

QVariant CSVFile::toVariant(const QString& s, bool wasQuoted)
{
    if (s.count('.') == 2)
    {
        QDate date = QDate::fromString(s, "dd.MM.yyyy");
        if (date.isValid())
        {
            return date;
        }
    }
    else if (s.count('/') == 2)
    {
        QDate date = QDate::fromString(s, "MM/dd/yyyy");
        if (date.isValid())
        {
            return date;
        }
    }

    if (wasQuoted)
    {
        return s.trimmed();
    }
    else
    {
        bool ok;
        int i;
        i = s.toInt(&ok);
        if (ok)
        {
            return i;
        }

        return s.trimmed();
    }
}
