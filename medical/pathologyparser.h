/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 28.06.2015
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

#ifndef PATHOLOGYPARSER_H
#define PATHOLOGYPARSER_H

#include "patient.h"
#include "property.h"

class QTextStream;

class PatientParseResults
{
public:

    PatientParseResults();
    PatientParseResults(const Patient& data);
    PatientParseResults(Patient::Ptr p);

    Patient patientData;
    Patient::Ptr p;
    QDate resultsDate;
    QStringList referenceNumbers;
    QString text;
    PropertyList properties;
    QString unrecognizedText;

    bool operator==(const Patient& otherPatientData) const;
};

class PathologyParser
{
public:
    PathologyParser();
    ~PathologyParser();

    /// Parses the given string / the text file. Current results are returned for convenience, or are available from results().
    QList<PatientParseResults> parse(const QString& s);
    QList<PatientParseResults> parseFile(const QString& fileName);
    /// Returns results from previous calls to parse()
    QList<PatientParseResults> results() const;

    /// Clears the stored results
    void clear();

private:

    void splitPerPatient(QTextStream& s);
    void parsePerPatient();
    void parseText(PatientParseResults& results);
    QList<Property> parseNGSText(const QString& protein, const QString& text);

    class PathologyParserPriv;
    PathologyParserPriv* const d;
};

#endif // PATHOLOGYPARSER_H
