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

#ifndef CSVFILE_H
#define CSVFILE_H


#include <QFile>
#include <QString>
#include <QTextStream>
#include <QVariant>

class CSVFile
{
public:

    CSVFile(const QChar& delimiter = ';');

    bool read(const QString& filePath);
    bool openForWriting(const QString& filePath);
    void finishWriting();
    void writeToString(QString *string);

    // Reading
    QList<QVariant> parseNextLine();
    bool atEnd() const;

    // Writing
    // write a line stepwise. Finish line by calling newLine
    CSVFile& operator<<(const QVariant& record);
    // finish the current line
    void newLine();
    // write a line in one go
    void writeNextLine(const QList<QVariant>& records);

private:

    QVariant toVariant(const QString& s, bool wasQuoted);

    QChar m_delimiter;
    QFile m_file;
    QTextStream m_stream;
    QList<QVariant> m_buffer;
};


#endif // CSVFILE_H
