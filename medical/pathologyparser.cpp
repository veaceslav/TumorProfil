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

#include "pathologyparser.h"

#include <boost/icl/interval_set.hpp>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>

#include "ihcscore.h"
#include "pathologypropertyinfo.h"
#include "patientmanager.h"

PatientParseResults::PatientParseResults()
{
}

PatientParseResults::PatientParseResults(const Patient& data)
    : patientData(data)
{
}

PatientParseResults::PatientParseResults(Patient::Ptr p)
    : patientData(*p),
      patient(p)
{
}

static bool isSamePatient(const Patient& a, const Patient& b)
{
    // Ignores gender (may be unknown)
    return a.surname == b.surname &&
            a.firstName == b.firstName &&
            a.dateOfBirth == b.dateOfBirth;
}

bool PatientParseResults::operator==(const Patient& other) const
{
    if (patient)
    {
        return isSamePatient(*patient, other);
    }
    return isSamePatient(patientData, other);
}


class RegExpContainer
{
public:

    enum RegExpVariant
    {
        InvalidRegExp,
        RegExpPatientId,
        RegExpIHC,
        RegExpIHCHScore,
        RegExpIHCMSI,
        RegExpNGSTableHeader,
        RegExpNGSTableRow,
        RegExpFISH,
        RegExpIgnore,
        RegExpPageBreakJunk,
        RegExpPDL1Header,
        RegExpPDL1ImmuneReaction,
        RegExpPDL1HScore,
        RegExpPDL1HScoreOld
    };

    QList<QRegularExpression>& expressions(RegExpVariant var)
    { return regexps[var]; }

    void load();

private:
    QMap<RegExpVariant, QList<QRegularExpression> > regexps;
};

void RegExpContainer::load()
{
    QMap<QString, RegExpVariant> keywords;
    keywords.insert("patient-id", RegExpPatientId);
    keywords.insert("ihc-global", RegExpIHC);
    keywords.insert("ihc-hscore", RegExpIHCHScore);
    keywords.insert("ihc-msi", RegExpIHCMSI);
    keywords.insert("ngs-table-header", RegExpNGSTableHeader);
    keywords.insert("ngs-table-row", RegExpNGSTableRow);
    keywords.insert("fish", RegExpFISH);
    keywords.insert("ignore", RegExpIgnore);
    keywords.insert("page-break-junk", RegExpPageBreakJunk);
    keywords.insert("", RegExpPDL1Header);
    keywords.insert("", RegExpPDL1ImmuneReaction);
    keywords.insert("", RegExpPDL1HScore);
    keywords.insert("", RegExpPDL1HScoreOld);

    // allow override with updated regexps in the current dir
    QFile file(QDir::currentPath() + "/pathology-regexps");
    if (!file.exists())
    {
        file.setFileName(":/regexps/pathology-regexps");
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open regexp file" << file.fileName() <<" - Parsing will not work!";
    }
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    QString line;
    RegExpVariant currentVariant = InvalidRegExp;
    while ( !(line = stream.readLine()).isNull() )
    {
        if (keywords.contains(line))
        {
            currentVariant = keywords.value(line);
        }
        else if (currentVariant != InvalidRegExp)
        {
            QRegularExpression re(line, QRegularExpression::MultilineOption | QRegularExpression::UseUnicodePropertiesOption);
            if (!re.isValid())
            {
                qDebug() << "Regexp" << line << "is invalid" << re.errorString() << re.patternErrorOffset() << line.mid(re.patternErrorOffset());
                continue;
            }
            if (currentVariant == RegExpFISH)
            {
                re.setPatternOptions(re.patternOptions() | QRegularExpression::DotMatchesEverythingOption);
            }
            regexps[currentVariant] << re;
        }
        else
        {
            qDebug() << "Regexp loading: failed to parse line" << line;
        }
    }
}

class PathologyParser::PathologyParserPriv
{
public:
    PathologyParserPriv()
    {
    }

    PatientParseResults& findResults(const Patient& patientData);
    void appendPatientText(PatientParseResults* results, const QString& chunk);
    IHCScore::Intensity textToIntensity(const QString& text);
    PathologyPropertyInfo::Property textToIHCProperty(const QString& protein);
    QList<Property> propertiesForProtein(const QString& protein);
    Property& propertyForExon(QList<Property>& properties, const QString& protein, int exon);
    PathologyPropertyInfo::Property textToFISHProperty(const QString& protein);

    RegExpContainer regExpContainer;
    QList<PatientParseResults> results;
};

PathologyParser::PathologyParser()
    : d(new PathologyParserPriv)
{
    d->regExpContainer.load();
}

PathologyParser::~PathologyParser()
{
    delete d;
}

QList<PatientParseResults> PathologyParser::results() const
{
    return d->results;
}

void PathologyParser::clear()
{
    d->results.clear();
}

QList<PatientParseResults> PathologyParser::parseFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
    {
        qDebug() << "Failed to open file" << fileName;
    }
    QTextStream stream(&file);
    stream.setCodec("ISO 8859-1");
    splitPerPatient(stream);
    parsePerPatient();

    return d->results;
}

QList<PatientParseResults> PathologyParser::parse(const QString& s)
{
    QString text(s);
    QTextStream stream(&text, QIODevice::ReadOnly);
    splitPerPatient(stream);
    parsePerPatient();

    return d->results;
}

void PathologyParser::splitPerPatient(QTextStream& stream)
{
    // First tour of parsing: Split into per-patient chunks.
    // We assume that the RegExps is exact (^...$)
    QString line, nextLine, patientText;
    PatientParseResults* currentResults = 0;
    while ( !(line = stream.readLine()).isNull() )
    {
        foreach (const QRegularExpression& re, d->regExpContainer.expressions(RegExpContainer::RegExpPatientId))
        {
            QRegularExpressionMatch match = re.match(line, 0, QRegularExpression::PartialPreferFirstMatch);
            // patient id is multi-line
            while (match.hasPartialMatch() && !(nextLine = stream.readLine()).isNull() )
            {
                line += "\n" + nextLine;
                match = re.match(line, 0, QRegularExpression::PartialPreferCompleteMatch);
            }
            if (match.hasMatch())
            {
                d->appendPatientText(currentResults, patientText);
                patientText.clear();

                Patient patientData;
                patientData.surname = match.captured("name");
                patientData.firstName = match.captured("firstName");
                patientData.dateOfBirth = QDate(match.captured("year").toInt(), match.captured("month").toInt(), match.captured("day").toInt());
                currentResults = &d->findResults(patientData);
                break;
            }
        }
        patientText += line + "\n";
    }
    d->appendPatientText(currentResults, patientText);
}

void PathologyParser::parsePerPatient()
{
    // The text is now broken into per-patient chunks, stored in results.
    // for each patient, parse their results, Properties and metadata are stored in results.
    for (int i=0; i<d->results.size(); ++i)
    {
        parseText(d->results[i]);
    }
}

void PathologyParser::parseText(PatientParseResults& results)
{
    boost::icl::interval_set<int> excerpts;
    results.resultsDate = QDate();
    results.referenceNumbers.clear();
    results.properties.clear();
    results.unrecognizedText.clear();

    // There is junk at the page break of the PDFs. This may be placed just inside any content, confusing the analysis.
    // So remove it here once and forever (regexp will be very specific)
    foreach (const QRegularExpression& re, d->regExpContainer.expressions(RegExpContainer::RegExpPageBreakJunk))
    {
        results.text.remove(re);
    }

    // we perform global matches with the (very specific) regular expressions

    // Look for id strings, fill finding date (earliest) and R/E numbers
    foreach (const QRegularExpression& re, d->regExpContainer.expressions(RegExpContainer::RegExpPatientId))
    {
        QRegularExpressionMatchIterator it = re.globalMatch(results.text);
        while (it.hasNext())
        {
            QRegularExpressionMatch match = it.next();
            QDate date(match.captured("refyear").toInt(), match.captured("refmonth").toInt(), match.captured("refday").toInt());
            if (date.isValid() && (results.resultsDate.isNull() || date < results.resultsDate))
            {
                results.resultsDate = date;
            }
            QString number = match.captured("refNumber");
            if (!number.isEmpty() && !results.referenceNumbers.contains(number))
            {
                results.referenceNumbers << number;
            }
            excerpts += boost::icl::interval<int>::right_open(match.capturedStart(), match.capturedEnd());
        }
    }
    foreach (const QRegularExpression& re, d->regExpContainer.expressions(RegExpContainer::RegExpIHC))
    {
        QRegularExpressionMatchIterator it = re.globalMatch(results.text);
        while (it.hasNext())
        {
            QRegularExpressionMatch match = it.next();
            // Some regexps may match the same line. Skip if this excerpt was already parsed.
            if (boost::icl::contains(excerpts, boost::icl::interval<int>::right_open(match.capturedStart(), match.capturedEnd())))
            {
                continue;
            }
            IHCScore::Intensity intensity = d->textToIntensity(match.captured("intensity"));
            PathologyPropertyInfo::Property id = d->textToIHCProperty(match.captured("protein"));
            PathologyPropertyInfo info = PathologyPropertyInfo::info(id);
            ValueTypeCategoryInfo typeInfo(info);

            Property prop;
            prop.property = info.id;
            typeInfo.fillIHCScore(prop, intensity, match.captured("percentage").remove('%'));

            results.properties << prop;

            if (id == PathologyPropertyInfo::IHC_HER2)
            {
                PathologyPropertyInfo dakoInfo = PathologyPropertyInfo::info(PathologyPropertyInfo::IHC_HER2_DAKO);
                ValueTypeCategoryInfo dakoTypeInfo(dakoInfo);
                Property dakoProp;
                dakoProp.property = dakoInfo.id;
                dakoProp.value    = dakoTypeInfo.toPropertyValue(match.captured("score").toInt());
                results.properties << dakoProp;
            }

            excerpts += boost::icl::interval<int>::right_open(match.capturedStart(), match.capturedEnd());
        }
    }
    foreach (const QRegularExpression& re, d->regExpContainer.expressions(RegExpContainer::RegExpIHCHScore))
    {
        QRegularExpressionMatchIterator it = re.globalMatch(results.text);
        while (it.hasNext())
        {
            QRegularExpressionMatch match = it.next();
            // the capture is empty for "Keine", which then correctly amounts to 0.
            HScore score(match.captured("strong").toInt(), match.captured("medium").toInt(), match.captured("weak").toInt());
            PathologyPropertyInfo::Property id = d->textToIHCProperty(match.captured("protein"));
            PathologyPropertyInfo info = PathologyPropertyInfo::info(id);
            ValueTypeCategoryInfo typeInfo(info);
            Property prop;
            prop.property = info.id;
            typeInfo.fillHSCore(prop, score);
            results.properties << prop;

            if (id == PathologyPropertyInfo::IHC_PDL1)
            {
                id = PathologyPropertyInfo::IHC_PDL1_immunecell;
                PathologyPropertyInfo info = PathologyPropertyInfo::info(id);
                ValueTypeCategoryInfo typeInfo(info);
                Property prop;
                prop.property = info.id;
                prop.value    = typeInfo.toPropertyValue(!match.captured("immunecell").contains("keine"));
                results.properties << prop;
            }

            excerpts += boost::icl::interval<int>::right_open(match.capturedStart(), match.capturedEnd());
        }
    }
    foreach (const QRegularExpression& re, d->regExpContainer.expressions(RegExpContainer::RegExpIHCMSI))
    {
        QRegularExpressionMatchIterator it = re.globalMatch(results.text);
        while (it.hasNext())
        {
            QRegularExpressionMatch match = it.next();
            PathologyPropertyInfo::Property id = d->textToIHCProperty(match.captured("protein"));

            PathologyPropertyInfo info = PathologyPropertyInfo::info(id);
            ValueTypeCategoryInfo typeInfo(info);
            Property prop;
            prop.property = info.id;
            QString positivityClause = match.captured("positivity");
            IHCScore score;
            if (positivityClause == "Ein" || positivityClause == "Kompletter")
            {
                typeInfo.fillIHCScore(prop, IHCScore::NoIntensity, QString());
            }
            else if (positivityClause == "Partieller")
            {
                typeInfo.fillIHCScore(prop, IHCScore::WeakIntensity, match.captured("percentage"));
            }
            else
            {
                typeInfo.fillIHCScore(prop, IHCScore::StrongIntensity, QString());
            }
            results.properties << prop;

            excerpts += boost::icl::interval<int>::right_open(match.capturedStart(), match.capturedEnd());
        }
    }
    foreach (const QRegularExpression& re, d->regExpContainer.expressions(RegExpContainer::RegExpNGSTableHeader))
    {
        QRegularExpressionMatchIterator it = re.globalMatch(results.text);
        while (it.hasNext())
        {
            QRegularExpressionMatch match = it.next();
            QString protein, mutationText;

            // We have no specific reg exp for the table data or its end.
            // So find the beginning of the next excerpt (next part already read) and read everything till there
            const int begin = match.capturedEnd();
            int end = results.text.size();
            for (boost::icl::interval_set<int>::const_iterator eit = excerpts.begin(); eit != excerpts.end(); ++eit)
            {
                if (eit->lower() > begin && eit->lower() < end)
                {
                    end = eit->lower();
                }
            }

            // get the subtext and read line-by-line
            foreach (const QString& line, results.text.mid(begin, end-begin).split('\n', QString::SkipEmptyParts))
            {
                QRegularExpressionMatch match2;
                foreach (const QRegularExpression& re, d->regExpContainer.expressions(RegExpContainer::RegExpNGSTableRow))
                {
                    match2 = re.match(line);
                    if (match2.hasMatch())
                    {
                        break;
                    }
                }

                if (match2.hasMatch())
                {
                    // scan last text, now complete
                    if (!protein.isEmpty())
                    {
                        results.properties += parseNGSText(protein, mutationText);
                    }
                    // start next round
                    protein = match2.captured("protein");
                    mutationText = line.mid(match2.capturedEnd());
                }
                else
                {
                    mutationText += line + '\n';
                }
            }
            // reached end, scan remaining, last mutation text
            if (!protein.isEmpty())
            {
                results.properties += parseNGSText(protein, mutationText);
            }
            excerpts += boost::icl::interval<int>::right_open(match.capturedStart(), end);
        }
    }
    foreach (const QRegularExpression& re, d->regExpContainer.expressions(RegExpContainer::RegExpFISH))
    {
        QRegularExpressionMatchIterator it = re.globalMatch(results.text);
        while (it.hasNext())
        {
            QRegularExpressionMatch match = it.next();
            PathologyPropertyInfo id = d->textToFISHProperty(match.captured("protein"));
            ValueTypeCategoryInfo typeInfo(id);
            Property prop;
            prop.property = id.id;
            QString positivityClause = match.captured("positivity");
            bool isPositive = (positivityClause == "positiv" || positivityClause == "mehr" || positivityClause == "eine" || positivityClause == "eine grenzwertige");
            prop.value      = typeInfo.toPropertyValue(isPositive);
            prop.detail     = match.captured("ratio");
            results.properties << prop;

            excerpts += boost::icl::interval<int>::right_open(match.capturedStart(), match.capturedEnd());
        }
    }
    foreach (const QRegularExpression& re, d->regExpContainer.expressions(RegExpContainer::RegExpIgnore))
    {
        QRegularExpressionMatchIterator it = re.globalMatch(results.text);
        while (it.hasNext())
        {
            QRegularExpressionMatch match = it.next();
            excerpts += boost::icl::interval<int>::right_open(match.capturedStart(), match.capturedEnd());
        }
    }

    /*qDebug() << results.patientData.firstName << results.patientData.surname << results.resultsDate << results.referenceNumbers;
    foreach (const Property& prop, results.properties)
    {
        qDebug() << prop.property << prop.value << prop.detail;
    }*/

    boost::icl::interval_set<int> unread;
    unread += boost::icl::interval<int>::right_open(0, results.text.size());
    unread -= excerpts;
    QStringList unrecognizedParts;
    for (boost::icl::interval_set<int>::const_iterator eit = unread.begin(); eit != unread.end(); ++eit)
    {
        unrecognizedParts << results.text.mid(eit->lower(), eit->upper() - eit->lower()).trimmed();
    }
    results.unrecognizedText = unrecognizedParts.join('\n');

    results.guessedEntity = Pathology::UnknownEntity;
    QMap<PathologyPropertyInfo::Property, Pathology::Entity> diseaseDefiningProperties;
    diseaseDefiningProperties.insert(PathologyPropertyInfo::IHC_ALK, Pathology::PulmonaryAdeno);
    diseaseDefiningProperties.insert(PathologyPropertyInfo::IHC_ROS1, Pathology::PulmonaryAdeno);
    diseaseDefiningProperties.insert(PathologyPropertyInfo::IHC_pP70S6K, Pathology::ColorectalAdeno);
    diseaseDefiningProperties.insert(PathologyPropertyInfo::Fish_FGFR1, Pathology::PulmonarySquamous);
    foreach (const Property& prop, results.properties)
    {
        PathologyPropertyInfo info = PathologyPropertyInfo::info(prop.property);
        if (diseaseDefiningProperties.contains(info.property))
        {
            results.guessedEntity = diseaseDefiningProperties.value(info.property);
            break;
        }
    }
}

QList<Property> PathologyParser::parseNGSText(const QString& protein, const QString& text)
{
    QList<Property> props = d->propertiesForProtein(protein);
    if (props.isEmpty())
    {
        return props;
    }

    // mutation negative?
    if (text.contains("negativ", Qt::CaseInsensitive))
    {
        // properties are prepared as "negative" in helper methods
        return props;
    }

    //TODO: Multiple exon handling with PDF. Rewrite.

    // mutation positive. Possibly, multiple exons.
    QRegularExpression exonRegExp("Exon (\\d+(?:\\s*,\\s*\\d+)*)");
    QRegularExpressionMatchIterator it = exonRegExp.globalMatch(text);
    while (it.hasNext())
    {
        QRegularExpressionMatch match = it.next();
        // get all text from the end of "Exon \d" to the next Exon line, or end of text
        QString mutationText = text.mid(match.capturedEnd(), it.hasNext() ? (it.peekNext().capturedStart() - match.capturedEnd()) : -1);

        QStringList exonNumber = match.captured().split(',');
        for (int i=0; i<exonNumber.size(); ++i)
        {
            Property& prop = d->propertyForExon(props, protein, exonNumber[i].toInt());
            ValueTypeCategoryInfo typeInfo(PathologyPropertyInfo::Mutation);
            prop.value = typeInfo.toPropertyValue(true);
            // HGVS code is well defined, parsable, but this is non-trivial.
            // There is a python library (called HGVS) available if needed.
            // For now, we copy.
            if (exonNumber.size() <= 1)
            {
                prop.detail = mutationText;
            }
            else
            {
                QString line;
                // TODO: Check for PDF. Dont know if this works.

                // If there are n exons, each exon has text every n'th line:
                for (int l=0; !(line = mutationText.section('\n', l, l, QString::SectionSkipEmpty | QString::SectionIncludeTrailingSep)).isNull();
                     l += exonNumber.size())
                {
                    prop.detail += line;
                }

            }
            // consolidate whitespace and \n's
            prop.detail.replace('\n', " ");
            prop.detail.replace(QRegularExpression("\\s{2,}"), " ");
            prop.detail.replace(" %", "%");
            prop.detail = prop.detail.trimmed();
        }
    }
    return props;
}

PatientParseResults& PathologyParser::PathologyParserPriv::findResults(const Patient& patientData)
{
    QList<PatientParseResults>::iterator it = std::find(results.begin(), results.end(), patientData);
    if (it != results.end())
    {
        return *it;
    }
    PatientParseResults r;
    QList<Patient::Ptr> ps= PatientManager::instance()->findPatients(patientData);
    if (!ps.isEmpty())
    {
        if (ps.size() > 1)
        {
            qDebug() << "Multiple candidates for patient" << patientData.firstName << patientData.surname << patientData.dateOfBirth;
        }
        results << PatientParseResults(ps.first());
    }
    else
    {
        results << PatientParseResults(patientData);
    }
    return results.last();
}

void PathologyParser::PathologyParserPriv::appendPatientText(PatientParseResults* results, const QString& chunk)
{
    if (results && !results->text.contains(chunk))
    {
        results->text += chunk;
        results->textPassages += chunk;
    }
}

QList<Property> PathologyParser::PathologyParserPriv::propertiesForProtein(const QString& protein)
{
    // Works in conjunction with propertyForExon
    /*
    BRAF    11,15
    DDR2    15-18
    EGFR    18,19,20,21
    ERBB2   5,6,15,20,23,29
    FGFR1   3,7,13,17
    FGFR3   7,9
    HRAS    2,3,4
    KIT     9,10,11,13,17,18
    KRAS    2,3,4
    MET     3,8,11,14,19
    NRAS    2,3,4
    PDGFRa  12,14,18
    PIK3CA  10,21
    RET     10,11,13,14,15,16
    TP53    4,5,6,7,8,9
    */
    QList<Property> props;
    QList<PathologyPropertyInfo> ids;
    if (protein == "BRAF")
    {
        ids << PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_BRAF_11);
        ids << PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_BRAF_15);
    }
    else if (protein == "DDR2")
    {
        ids << PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_DDR2);
    }
    else if (protein == "EGFR")
    {
        ids << PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_EGFR_19_21);
        ids << PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_EGFR_18_20);
    }
    else if (protein == "ERBB2")
    {
        ids << PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_ERBB2);
    }
    else if (protein == "FGFR1")
    {
        ids << PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_FGFR1);
    }
    else if (protein == "FGFR3")
    {
        ids << PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_FGFR3);
    }
    else if (protein == "HRAS")
    {
        ids << PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_HRAS_2_4);
    }
    else if (protein == "KIT")
    {
        ids << PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_KIT);
    }
    else if (protein == "KRAS")
    {
        ids << PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_KRAS_2);
        ids << PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_KRAS_3);
        ids << PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_KRAS_4);
    }
    else if (protein == "MET")
    {
        ids << PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_MET);
    }
    else if (protein == "NRAS")
    {
        ids << PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_NRAS_2_4);
    }
    else if (protein == "PDGFRa")
    {
        ids << PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_PDGFRa);
    }
    else if (protein == "PIK3CA")
    {
        ids << PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_PIK3CA_10_21);
    }
    else if (protein == "RET")
    {
        ids << PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_RET);
    }
    else if (protein == "TP53")
    {
        ids << PathologyPropertyInfo::info(PathologyPropertyInfo::Mut_TP53);
    }
    else
    {
        qDebug() << "Error: No property known for protein" << protein;
    }

    foreach (PathologyPropertyInfo id, ids)
    {
        ValueTypeCategoryInfo typeInfo(id);
        Property prop;
        prop.property = id.id;
        // Prepare as a negative value
        prop.value = typeInfo.toPropertyValue(typeInfo.negativeValue());
        props << prop;
    }
    return props;
}

Property& PathologyParser::PathologyParserPriv::propertyForExon(QList<Property>& properties, const QString& protein, int exon)
{
    // this is the check-free simple solution for the majority of cases
    if (properties.size() == 1)
    {
        return properties.first();
    }
    if (protein == "BRAF")
    {
        if (exon == 11)
        {
            return properties[0];
        }
        else
        {
            return properties[1];
        }
    }
    else if (protein == "EGFR")
    {
        switch (exon)
        {
        case 19:
        case 21:
        default:
            return properties[0];
        case 18:
        case 20:
            return properties[1];
        }
    }
    else if (protein == "KRAS")
    {
        switch (exon)
        {
        case 2:
        default:
            return properties[0];
        case 3:
            return properties[1];
        case 4:
            return properties[2];
        }
    }

    qDebug() << "propertyForExon: unhandled case" << protein << exon << " returning first";
    // in the end we return the first property, also if in doubt
    return properties.first();
}

IHCScore::Intensity PathologyParser::PathologyParserPriv::textToIntensity(const QString& text)
{
    // (Schwache|Mäßige|Starke|Keine)
    if (text == "Schwache")
    {
        return IHCScore::WeakIntensity;
    }
    if (text == "Mäßige")
    {
        return IHCScore::MediumIntensity;
    }
    if (text == "Starke" || text == "Kräftige")
    {
        return IHCScore::StrongIntensity;
    }
    if (text == "Keine")
    {
        return IHCScore::NoIntensity;
    }
    return IHCScore::InvalidIntensity;
}

PathologyPropertyInfo::Property PathologyParser::PathologyParserPriv::textToIHCProperty(const QString& protein)
{
    // PTEN|c-MET|p-ERK|ALK|ROS1|p-AKTS473|HER2\/neu|p-p70S6-Kinase
    if (protein == "PTEN")
    {
        return PathologyPropertyInfo::IHC_PTEN;
    }
    if (protein == "c-MET")
    {
        return PathologyPropertyInfo::IHC_cMET;
    }
    if (protein == "p-ERK")
    {
        return PathologyPropertyInfo::IHC_pERK;
    }
    if (protein == "ALK")
    {
        return PathologyPropertyInfo::IHC_ALK;
    }
    if (protein == "ROS1")
    {
        return PathologyPropertyInfo::IHC_ROS1;
    }
    if (protein == "p-AKTS473" || protein == "p-AKT")
    {
        return PathologyPropertyInfo::IHC_pAKT;
    }
    if (protein == "HER2/neu")
    {
        return PathologyPropertyInfo::IHC_HER2;
    }
    if (protein == "p-p70S6-Kinase" || protein == "p-p70s6-Kinase")
    {
        return PathologyPropertyInfo::IHC_pP70S6K;
    }
    if (protein == "c-MET")
    {
        return PathologyPropertyInfo::IHC_cMET;
    }
    if (protein == "PD-L1")
    {
        return PathologyPropertyInfo::IHC_PDL1;
    }
    if (protein == "MLH1")
    {
        return PathologyPropertyInfo::IHC_MLH1;
    }
    if (protein == "MSH2")
    {
        return PathologyPropertyInfo::IHC_MSH2;
    }
    if (protein == "MSH6")
    {
        return PathologyPropertyInfo::IHC_MSH6;
    }
    qDebug() << "Unhandled IHC" << protein << "in scanner";
    return PathologyPropertyInfo::InvalidProperty;
}

PathologyPropertyInfo::Property PathologyParser::PathologyParserPriv::textToFISHProperty(const QString& protein)
{
    // three different regexps for four FISHes
    if (protein == "HER2")
    {
        return PathologyPropertyInfo::Fish_HER2;
    }
    if (protein == "ALK")
    {
        return PathologyPropertyInfo::Fish_ALK;
    }
    if (protein == "ROS1")
    {
        return PathologyPropertyInfo::Fish_ROS1;
    }
    if (protein == "FGFR1")
    {
        return PathologyPropertyInfo::Fish_FGFR1;
    }
    qDebug() << "Unhandled FISH" << protein << "in scanner";
    return PathologyPropertyInfo::InvalidProperty;
}
