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

#include "diseasehistory.h"

// C++ includes

#include <algorithm>

// Qt includes

#include <QDebug>
#include <QMap>
#include <QMultiHash>
#include <QPair>
#include <QRegularExpression>

// Local includes

#include "databaseconstants.h"
#include "diseasehistory.h"
#include "xmlstreamutils.h"
#include "xmltextintmapper.h"

class DiseaseHistory::Private : public QSharedData
{
public:
    Private()
    {
        qDeleteAll(history);
    }

    ~Private()
    {
    }

    HistoryElementList history;
    PropertyList       properties;
    QList<Event>                                  unknownEvents;
    QMultiHash<const  HistoryElement*, EventInfo> unknownEventInfos;


    static QLatin1String lastDocumentationPropertyName()
    {
        return QLatin1String("lastDocumentation");
    }

    static QLatin1String lastValidationPropertyName()
    {
        return QLatin1String("lastValidation");
    }

    void addUnknownEventInfos(const HistoryElement* e, Event& event) const
    {
        QHash<const HistoryElement*, EventInfo>::const_iterator it = unknownEventInfos.find(e);
        while (it != unknownEventInfos.end() && it.key() == e)
        {
            event.infos << it.value();
        }
    }
};

// -----------------------------------------------------------------------------------------------

class PrivateSharedNull : public QSharedDataPointer<DiseaseHistory::Private>
{
public:

    PrivateSharedNull()
        : QSharedDataPointer<DiseaseHistory::Private>(new DiseaseHistory::Private)
    {
    }
};

Q_GLOBAL_STATIC(PrivateSharedNull, diseaseHistoryPrivSharedNull)

// -----------------------------------------------------------------------------------------------

DiseaseHistory::DiseaseHistory()
    : d(*diseaseHistoryPrivSharedNull())
{
}

DiseaseHistory::DiseaseHistory(const DiseaseHistory &other)
{
    d = other.d;
}

DiseaseHistory::~DiseaseHistory()
{
}

DiseaseHistory& DiseaseHistory::operator=(const DiseaseHistory& other)
{
    d = other.d;
    return *this;
}

bool DiseaseHistory::isEmpty() const
{
    return d->history.isEmpty();
}

int  DiseaseHistory::size() const
{
    return d->history.size();
}

QDate DiseaseHistory::begin() const
{
    QDate earliest;
    foreach (HistoryElement* e, d->history)
    {
        if (earliest.isNull() || e->date < earliest)
        {
            earliest = e->date;
        }
    }
    return earliest;
}

QDate DiseaseHistory::end() const
{
    QDate earliest;
    foreach (HistoryElement* e, d->history)
    {
        if (earliest.isNull() || e->date > earliest)
        {
            earliest = e->date;
        }
    }
    return earliest;
}

DiseaseState DiseaseHistory::state() const
{
    const DiseaseState* state = d->history.first<DiseaseState>();
    if (state)
    {
        return *state;
    }
    return DiseaseState();
}

const HistoryElementList& DiseaseHistory::entries() const
{
    return d->history;
}

bool DiseaseHistory::operator==(const DiseaseHistory& other) const
{
    return other.toXml() == toXml();
}

HistoryElement* DiseaseHistory::operator[](int i)
{
    return d->history[i];
}

const HistoryElement* DiseaseHistory::operator[](int i) const
{
    return d->history[i];
}

DiseaseHistory& DiseaseHistory::operator<<(HistoryElement* e)
{
    d->history << e;
    return *this;
}

QDate DiseaseHistory::latestDate() const
{
    if (isEmpty())
    {
        return QDate();
    }
    return d->history.latestByDate()->date;
}

void DiseaseHistory::insert(int place, HistoryElement *e)
{
    if (!e)
    {
        return;
    }
    d->history.insert(place, e);
}

void DiseaseHistory::remove(HistoryElement* e)
{
    if (!e)
    {
        return;
    }
    if (e->parent())
    {
        if (e->is<TherapyElement>())
        {
            TherapyElement* te = e->as<TherapyElement>();
            Therapy* t = e->parent()->as<Therapy>();
            t->elements.removeAll(te);
            te->setParent(0);
        }
    }
    else
    {
        d->history.removeAll(e);
        d->unknownEventInfos.remove(e);
    }
}

PropertyList& DiseaseHistory::properties()
{
    return d->properties;
}

const PropertyList& DiseaseHistory::properties() const
{
    return d->properties;
}

void DiseaseHistory::setLastDocumentation(const QDate& date)
{
    if (!date.isValid())
    {
        d->properties.removeProperty(d->lastDocumentationPropertyName());
    }
    else
    {
        d->properties.setProperty(d->lastDocumentationPropertyName(), date.toString(Qt::ISODate));
    }
}

QDate DiseaseHistory::lastDocumentation() const
{
    Property prop = d->properties.property(d->lastDocumentationPropertyName());
    if (prop.isNull())
    {
        return QDate();
    }
    return QDate::fromString(prop.value, Qt::ISODate);
}

void DiseaseHistory::setLastValidation(const QDate& date)
{
    if (!date.isValid())
    {
        d->properties.removeProperty(d->lastValidationPropertyName());
    }
    else
    {
        d->properties.setProperty(d->lastValidationPropertyName(), date.toString(Qt::ISODate));
    }
}

QDate DiseaseHistory::lastValidation() const
{
    Property prop = d->properties.property(d->lastValidationPropertyName());
    if (prop.isNull())
    {
        return QDate();
    }
    return QDate::fromString(prop.value, Qt::ISODate);
}

static int historyElementTypeOrder(const HistoryElement* a)
{
    return a->is<Finding>() ? 1
         : a->is<Therapy>() ? 2
         : a->is<Finding>() ? 3
         : 4;
}

static bool lessThanForHistoryElements(const HistoryElement* a, const HistoryElement* b)
{
    if (a->date != b->date)
    {
        return a->date < b->date;
    }
    return historyElementTypeOrder(a) < historyElementTypeOrder(b);
}

void DiseaseHistory::sort()
{
    qStableSort(d->history.begin(), d->history.end(), lessThanForHistoryElements);
}

bool DiseaseHistory::isSorted() const
{
    return std::is_sorted(d->history.begin(), d->history.end(), lessThanForHistoryElements);
}

int DiseaseHistory::sortPlace(HistoryElement *element) const
{
    // method makes only sense if the list is sorted

    // we must use the exact same algorithm as in sort() to have reliable results
    HistoryElementList copy = d->history;
    if (!copy.contains(element))
    {
        copy << element;
    }
    qStableSort(copy.begin(), copy.end(), lessThanForHistoryElements);
    return copy.indexOf(element);
}

TEXT_INT_MAPPER(Therapy, Type)
{
    Pair("ctx", Therapy::CTx),
    Pair("rtx", Therapy::RTx),
    Pair("rctx", Therapy::RCTx),
    Pair("surgery", Therapy::Surgery),
    Pair("intervention", Therapy::Intervention),
};

TEXT_INT_MAPPER(Therapy, AdditionalInfo)
{
    Pair("infoBeginsTherapyBlock", Therapy::BeginsTherapyBlock),
    Pair("infoEndsTherapyBlock", Therapy::EndsTherapyBlock)
};

TEXT_INT_MAPPER(Finding, Type)
{
    Pair("clinical", Finding::Clinical),
    Pair("histopathological", Finding::Histopathological),
    Pair("death", Finding::Death)
};

TEXT_INT_MAPPER(Finding, Modality)
{
    Pair("ct", Finding::CT),
    Pair("mri", Finding::MRI),
    Pair("xray", Finding::XRay),
    Pair("sono", Finding::Sono),
    Pair("pet-ct", Finding::PETCT),
    Pair("scintigraphy", Finding::Scintigraphy)
};

TEXT_INT_MAPPER(Finding, Result)
{
    Pair("sd", Finding::SD),
    Pair("pd", Finding::PD),
    Pair("mr", Finding::MR),
    Pair("pr", Finding::PR),
    Pair("cr", Finding::CR),
    Pair("ned", Finding::NED),
    Pair("initial", Finding::InitialFindingResult),
    Pair("recurrence", Finding::Recurrence),
    Pair("n-a", Finding::ResultNotApplicable)
};

TEXT_INT_MAPPER(Finding, Context)
{
    Pair("antecedent", Finding::Antecedent),
    Pair("initialDiagnosis", Finding::InitialDiagnosis),
    Pair("responseEvaluation", Finding::ResponseEvaluation),
    Pair("followUp", Finding::FollowUp),
};

TEXT_INT_MAPPER(Finding, AdditionalInfo)
{
    Pair("infoLocalRecurrence", Finding::LocalRecurrence),
    Pair("infoMetastasis", Finding::Metastasis),
    Pair("  ", Finding::CentralNervous),
};

TEXT_INT_MAPPER(DiseaseState, State)
{
    Pair("initialDiagnosis", DiseaseState::InitialDiagnosis),
    Pair("therapy", DiseaseState::Therapy),
    Pair("bestSupportiveCare", DiseaseState::BestSupportiveCare),
    Pair("followUp", DiseaseState::FollowUp),
    Pair("deceased", DiseaseState::Deceased),
    Pair("lossOfContact", DiseaseState::LossOfContact),
    Pair("watchAndWait", DiseaseState::WatchAndWait)
};

QString DiseaseHistory::toXml() const
{
    QString xmlHistory;

    XmlStreamWriter stream(&xmlHistory);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();
    stream.writeStartElement("history");
    stream.writeAttribute("version", QString::number(1));

    foreach (const Property& prop, d->properties)
    {
        stream.writeStartElement("property");
        stream.writeAttribute("name", prop.property);
        if (!prop.value.isEmpty())
        {
            stream.writeAttribute("value", prop.value);
        }
        if (!prop.detail.isEmpty())
        {
            stream.writeAttribute("detail", prop.detail);
        }
        stream.writeEndElement();
    }

    foreach (const HistoryElement* e, d->history)
    {
        if (e->is<Therapy>())
        {
            const Therapy* t = static_cast<const Therapy*>(e);
            stream.writeStartElement("therapy");
            stream.writeAttribute("type", TherapyTypeTextIntMapper::toString(t->type));
            stream.writeAttribute("begin", t->date);
            stream.writeAttributeChecked("end", t->end);
            stream.writeAttributeChecked("bestResponse", FindingResultTextIntMapper::toString(t->bestResponse));
            stream.writeAttributeChecked("outcome", FindingResultTextIntMapper::toString(t->outcome));
            stream.writeAttributeChecked("description", t->description);
            stream.writeFlagAttributes<Therapy::AdditionalInfo, TherapyAdditionalInfoTextIntMapper>
                    (t->additionalInfos, QList<Therapy::AdditionalInfo>()
                      << Therapy::BeginsTherapyBlock << Therapy::EndsTherapyBlock);
            foreach (TherapyElement* te, t->elements)
            {
                if (te->is<Chemotherapy>())
                {
                    const Chemotherapy* ctx = static_cast<const Chemotherapy*>(te);
                    stream.writeStartElement("chemotherapy");
                    stream.writeAttributeChecked("substance", ctx->substance);
                    stream.writeAttributeChecked("dose", ctx->dose);
                    stream.writeAttributeChecked("absdose", ctx->absdose);
                    stream.writeAttributeChecked("schedule", ctx->schedule);
                    stream.writeAttributeChecked("cycles", ctx->cycles);
                    stream.writeEndElement();
                }
                else if (te->is<Radiotherapy>())
                {
                    const Radiotherapy* rtx = static_cast<const Radiotherapy*>(te);
                    stream.writeStartElement("radiotherapy");
                    stream.writeAttributeChecked("location", rtx->location);
                    stream.writeAttributeChecked("dose", rtx->dose);
                    stream.writeEndElement();
                }
                else if (te->is<Toxicity>())
                {
                    const Toxicity* tox = static_cast<const Toxicity*>(te);
                    stream.writeStartElement("toxicity");
                    stream.writeAttributeChecked("description", tox->description);
                    stream.writeAttributeChecked("grade", tox->grade);
                    stream.writeEndElement();
                }
            }

            stream.writeEndElement(); // therapy
        }
        else if (e->is<Finding>())
        {
            const Finding* f = static_cast<const Finding*>(e);
            stream.writeStartElement("finding");
            // for historic reasons, in XML type and modality are stored in the same field
            if (f->type == Finding::Imaging)
            {
                stream.writeAttributeChecked("type", FindingModalityTextIntMapper::toString(f->modality));
            }
            else
            {
                stream.writeAttributeChecked("type", FindingTypeTextIntMapper::toString(f->type));
            }
            stream.writeAttributeChecked("type", FindingTypeTextIntMapper::toString(f->type));
            stream.writeAttributeChecked("context", FindingContextTextIntMapper::toString(f->context));
            stream.writeAttributeChecked("result", FindingResultTextIntMapper::toString(f->result));
            stream.writeAttributeChecked("date", f->date);
            stream.writeAttributeChecked("description", f->description);
            stream.writeFlagAttributes<Finding::AdditionalInfo, FindingAdditionalInfoTextIntMapper>
                    (f->additionalInfos, QList<Finding::AdditionalInfo>()
                      << Finding::LocalRecurrence << Finding::Metastasis << Finding::CentralNervous);
            stream.writeEndElement(); // finding
        }
        else if (e->is<DiseaseState>())
        {
            const DiseaseState* t = static_cast<const DiseaseState*>(e);
            stream.writeStartElement("diseasestate");
            stream.writeAttribute("state", DiseaseStateStateTextIntMapper::toString(t->state));
            stream.writeAttributeChecked("date", t->date);
            stream.writeEndElement(); // diseasestate
        }
    }

    stream.writeEndElement(); // history;
    stream.writeEndDocument();

    return xmlHistory;
}

DiseaseHistory DiseaseHistory::fromXml(const QString& xml)
{
    DiseaseHistory h;

    if (xml.isEmpty())
    {
        return h;
    }

    XmlStreamReader stream(xml);

    if (!stream.readNextStartElement())
    {
        return h;
    }

    if (stream.name() != "history")
    {
        return h;
    }

    while (stream.readNextStartElement())
    {
        if (stream.name() == "therapy")
        {
            Therapy* t = new Therapy;
            stream.readAttributeCheckedEnum<Therapy::Type, TherapyTypeTextIntMapper>("type", t->type);
            stream.readAttributeChecked("begin", t->date);
            stream.readAttributeChecked("end", t->end);
            stream.readAttributeChecked("description", t->description);
            stream.readFlagAttributes<Therapy::AdditionalInfo, TherapyAdditionalInfoTextIntMapper>
                    (t->additionalInfos, QList<Therapy::AdditionalInfo>()
                      << Therapy::BeginsTherapyBlock << Therapy::EndsTherapyBlock);

            while (stream.readNextStartElement())
            {
                if (stream.name() == "chemotherapy")
                {
                    Chemotherapy* ctx = new Chemotherapy;
                    stream.readAttributeChecked("substance", ctx->substance);
                    stream.readAttributeChecked("dose", ctx->dose);
                    stream.readAttributeChecked("absdose", ctx->absdose);
                    stream.readAttributeChecked("schedule", ctx->schedule);
                    stream.skipCurrentElement();
                    t->elements << ctx;
                }
                else if (stream.name() == "radiotherapy")
                {
                    Radiotherapy* rtx = new Radiotherapy;
                    stream.readAttributeChecked("location", rtx->location);
                    stream.readAttributeChecked("dose", rtx->dose);
                    stream.skipCurrentElement();
                    t->elements << rtx;
                }
                else if (stream.name() == "toxicity")
                {
                    Toxicity* tox = new Toxicity;
                    stream.readAttributeChecked("description", tox->description);
                    stream.readAttributeChecked("grade", tox->grade);
                    stream.skipCurrentElement();
                    t->elements << tox;
                }
                else
                {
                    stream.skipCurrentElement();
                }
            }

            h << t;
        }
        else if (stream.name() == "finding")
        {
            Finding* f = new Finding;
            // for historic reasons, in XML, we have type and modality in one field
            QString typeString;
            stream.readAttributeChecked("type", typeString);
            if (FindingModalityTextIntMapper::toEnum(typeString) != Finding::UndefinedModality)
            {
                f->type = Finding::Imaging;
                f->modality = FindingModalityTextIntMapper::toEnum(typeString);
            }
            else
            {
                f->type = FindingTypeTextIntMapper::toEnum(typeString);
            }
            stream.readAttributeCheckedEnum<Finding::Context, FindingContextTextIntMapper>("context", f->context);
            stream.readAttributeCheckedEnum<Finding::Result, FindingResultTextIntMapper>("result", f->result);
            stream.readAttributeChecked("date", f->date);
            stream.readAttributeChecked("description", f->description);
            stream.readFlagAttributes<Finding::AdditionalInfo, FindingAdditionalInfoTextIntMapper>
                    (f->additionalInfos, QList<Finding::AdditionalInfo>()
                      << Finding::LocalRecurrence << Finding::Metastasis << Finding::CentralNervous);
            stream.skipCurrentElement();

            h << f;
        }
        else if (stream.name() == "diseasestate")
        {
            DiseaseState* t = new DiseaseState;
            stream.readAttributeCheckedEnum<DiseaseState::State, DiseaseStateStateTextIntMapper>("state", t->state);
            stream.readAttributeChecked("date", t->date);
            stream.skipCurrentElement();

            h << t;
        }
        else if (stream.name() == "property")
        {
            Property prop;
            stream.readAttributeChecked("name", prop.property);
            stream.readAttributeChecked("value", prop.value);
            stream.readAttributeChecked("detail", prop.detail);
            stream.skipCurrentElement();
            h.properties() << prop;
        }
        else
        {
            qDebug() << "DiseaseHistory XML: Unhandled tier 1 element" << stream.name();
            stream.skipCurrentElement();
        }
    }

    if (stream.hasError())
    {
        //TODO: error handling
        qDebug() << "An error occurred during parsing: " << stream.errorString();
    }

    h.sort();
    return h;
}

EVENT_TEXT_INT_MAPPER(Therapy, Type)
{
    Pair("ctx", Therapy::CTx),
    Pair("rtx", Therapy::RTx),
    Pair("crtx", Therapy::RCTx),
    Pair("sx", Therapy::Surgery),
    Pair("intervention", Therapy::Intervention)
};

EVENT_TEXT_INT_MAPPER(Therapy, AdditionalInfo)
{
    Pair("begin", Therapy::BeginsTherapyBlock),
    Pair("end", Therapy::EndsTherapyBlock)
};

EVENT_TEXT_INT_MAPPER(DiseaseState, State)
{
    Pair("initialdx", DiseaseState::InitialDiagnosis),
    Pair("therapy", DiseaseState::Therapy),
    Pair("bsc", DiseaseState::BestSupportiveCare),
    Pair("followup", DiseaseState::FollowUp),
    Pair("deceased", DiseaseState::Deceased),
    Pair("ltfu", DiseaseState::LossOfContact),
    Pair("ww", DiseaseState::WatchAndWait),
    Pair("bestsupportivecare", DiseaseState::BestSupportiveCare),
    Pair("watchandwait", DiseaseState::WatchAndWait)
};

EVENT_TEXT_INT_MAPPER(Finding, Type)
{
    Pair("imaging", Finding::Imaging),
    Pair("clinical", Finding::Clinical),
    Pair("histopathological", Finding::Histopathological),
    Pair("death", Finding::Death)
};

EVENT_TEXT_INT_MAPPER(Finding, Modality)
{
    Pair("ct", Finding::CT),
    Pair("mri", Finding::MRI),
    Pair("xray", Finding::XRay),
    Pair("sono", Finding::Sono),
    Pair("pet-ct", Finding::PETCT),
    Pair("scintigraphy", Finding::Scintigraphy)
};

EVENT_TEXT_INT_MAPPER(Finding, Result)
{
    Pair("sd", Finding::SD),
    Pair("pd", Finding::PD),
    Pair("mr", Finding::MR),
    Pair("pr", Finding::PR),
    Pair("cr", Finding::CR),
    Pair("ned", Finding::NED),
    Pair("initial", Finding::InitialFindingResult),
    Pair("relapse", Finding::Recurrence),
    Pair("na", Finding::ResultNotApplicable)
};

EVENT_TEXT_INT_MAPPER(Finding, Context)
{
    Pair("antecedent", Finding::Antecedent),
    Pair("initialdx", Finding::InitialDiagnosis),
    Pair("eval", Finding::ResponseEvaluation),
    Pair("followup", Finding::FollowUp)
};

EVENT_TEXT_INT_MAPPER(Finding, AdditionalInfo)
{
    Pair("cns", Finding::CentralNervous),
    Pair("localrelapse", Finding::LocalRecurrence),
    Pair("firstmetastasis", Finding::Metastasis)
};

static QMap<QString, QString> splitByColonFields(const QString& s, const QStringList seps)
{
    QMap<QString, QString> map;
    QString pattern;
    foreach (const QString& sep, seps)
    {
        if (!pattern.isEmpty())
            pattern += '|';
        pattern += " ?"+sep+":";
    }

    QRegularExpression re(pattern);
    if (s.contains(re))
    {
        QStringList sections, sectionHeaders;
        QRegularExpressionMatchIterator it = re.globalMatch(s);
        while (it.hasNext())
        {
            QRegularExpressionMatch match = it.next();
            sectionHeaders << match.captured();
        }
        sections = s.split(re, QString::KeepEmptyParts);

        map["main"] = sections.first();
        for (int i=0;i<sectionHeaders.size() && i+1<sections.size();i++)
        {
            const QString& sectionHeader = sectionHeaders[i];
            const QString& section = sections[i+1];
            QString sep = sectionHeader.mid(0, sectionHeader.length()-1).trimmed();
            map[sep] = section;
        }
    }
    else
    {
        map["main"] = s;
    }
    return map;
}

static QString mergeWithColonFields(const QMap<QString,QString> &map)
{
    if (map.isEmpty())
    {
        return QString();
    }

    QString s = map.value("main");

    for (QMap<QString,QString>::const_iterator it = map.begin(); it != map.end(); ++it)
    {
        if (it.key() == "main")
        {
            continue;
        }
        if (!s.isEmpty())
        {
            s += ' ';
        }
        s += it.key();
        s += ":";
        s += it.value();
    }
    return s;
}

static QString fromNumber(int i)
{
    QString num;
    num.setNum(i);
    return num;
}

DiseaseHistory DiseaseHistory::fromEvents(const QList<Event>& events)
{
    DiseaseHistory h;

    if (events.isEmpty())
    {
        return h;
    }

    foreach (const Event& event, events)
    {
        if (event.eventClass == "therapy")
        {
            Therapy* t = new Therapy;
            t->type = EventTherapyTypeTextIntMapper(event.type);
            t->date = event.date;

            int ctxCycles = 0;

            foreach (const EventInfo& info, event.infos)
            {
                if (info.type == "therapy:end")
                {
                    t->end = QDate::fromString(info.info, Qt::ISODate);
                }
                else if (info.type == "therapy:ctx:substance")
                {
                    Chemotherapy* ctx = new Chemotherapy;
                    QMap<QString, QString> map =
                            splitByColonFields(info.info, QStringList() << "dose" << "absdose" << "schedule");
                    ctx->substance = map.value("main");
                    for (QMap<QString, QString>::const_iterator it = map.begin(); it != map.end(); ++it)
                    {
                        if (it.key() == "dose")
                        {
                            ctx->dose = it.value().toInt();
                        }
                        else if (it.key() == "absdose")
                        {
                            ctx->absdose = it.value().toInt();
                        }
                        else if (it.key() == "schedule")
                        {
                            ctx->schedule = it.value();
                        }
                    }

                    t->elements << ctx;
                }
                else if (info.type == "therapy:ctx:cycles")
                {
                    ctxCycles = info.info.toInt();
                }
                else if (info.type == "therapy:rtx:treatment")
                {
                    Radiotherapy* rtx = new Radiotherapy;
                    QMap<QString, QString> map =
                            splitByColonFields(info.info, QStringList() << "dose");
                    rtx->location = map.value("main");
                    for (QMap<QString, QString>::const_iterator it = map.begin(); it != map.end(); ++it)
                    {
                        if (it.key() == "dose")
                        {
                            rtx->dose = it.value().toInt();
                        }
                    }

                    t->elements << rtx;
                }
                else if (info.type == "therapy:toxicity")
                {
                    Toxicity* tox = new Toxicity;
                    QMap<QString, QString> map =
                            splitByColonFields(info.info, QStringList() << "grade");
                    tox->description = map.value("main");
                    for (QMap<QString, QString>::const_iterator it = map.begin(); it != map.end(); ++it)
                    {
                        if (it.key() == "grade")
                        {
                            tox->grade = it.value().toInt();
                        }
                    }
                    t->elements << tox;
                }
                else if (info.type == "therapy:outcome")
                {
                    t->outcome = EventFindingResultTextIntMapper(info.info);
                }
                else if (info.type == "therapy:bestresponse")
                {
                    t->bestResponse = EventFindingResultTextIntMapper(info.info);
                }
                else if (info.type == "therapy:comment")
                {
                    t->description = info.info;
                }
                else if (info.type == "therapy:block")
                {
                    // handles "begin" and "end"
                    t->additionalInfos |= EventTherapyAdditionalInfoTextIntMapper(info.info);
                }
                else
                {
                    qDebug() << "Unhandled event info with from event class therapy, type" << event.type << ", unknown info type" << info.type;
                    h.d->unknownEventInfos.insert(t, info);
                }
            }

            if (ctxCycles)
            {
                for (TherapyElementList::iterator it = t->elements.begin(); it != t->elements.end(); ++it)
                {
                    if ((*it)->is<Chemotherapy>())
                    {
                        (*it)->as<Chemotherapy>()->cycles = ctxCycles;
                    }
                }
            }

            h << t;
        }
        else if (event.eventClass == "finding")
        {
            Finding* f = new Finding;
            f->type = EventFindingTypeTextIntMapper(event.type);
            f->date = event.date;

            foreach (const EventInfo& info, event.infos)
            {
                if (info.type == "finding:assessment")
                {
                    f->result = EventFindingResultTextIntMapper(info.info);
                }
                else if (info.type == "finding:site")
                {
                    // will handle "cns"
                    f->additionalInfos |= EventFindingAdditionalInfoTextIntMapper(info.info);
                }
                else if (info.type == "finding:context")
                {
                    f->context = EventFindingContextTextIntMapper(info.info);
                }
                else if (info.type == "finding:event")
                {
                    // will handle "localrelapse", "firstmetastasis"
                    f->additionalInfos |= EventFindingAdditionalInfoTextIntMapper(info.info);
                }
                else if (info.type == "finding:imaging:modality")
                {
                    f->modality = EventFindingModalityTextIntMapper(info.info);
                }
                else if (info.type == "finding:comment")
                {
                    f->description = info.info;
                }
                else
                {
                    qDebug() << "Unhandled event info with from event class finding, type" << event.type << ", unknown info type" << info.type;
                    h.d->unknownEventInfos.insert(f, info);
                }
            }

            h << f;
        }
        else if (event.eventClass == "diseasestate")
        {
            DiseaseState* s = new DiseaseState;
            s->state = EventDiseaseStateStateTextIntMapper::toEnum(event.type);
            s->date  = event.date;

            foreach (const EventInfo& info, event.infos)
            {
                qDebug() << "Unhandled event info with from event class diseasestate, type" << event.type << "unknown info type" << info.type;
                h.d->unknownEventInfos.insert(s, info);
            }

            h << s;
        }
        else if (event.eventClass == "metadata")
        {
            if (event.type == "lastdocumentation")
            {
                h.setLastDocumentation(event.date);
            }
            else if (event.type == "lastvalidation")
            {
                h.setLastValidation(event.date);
            }
            else
            {
                qDebug() << "Unhandled event with class metadata, unknown type" << event.type;
                h.d->unknownEvents << event;
            }
        }
        else
        {
            qDebug() << "Unhandled event with unknown class" << event.eventClass << ", type" << event.type;
            h.d->unknownEvents << event;
        }

    }

    h.sort();
    return h;
}

QList<Event> DiseaseHistory::toEvents() const
{
    QList<Event> events;

    if (!lastDocumentation().isNull())
    {
        events << Event("metadata", "lastdocumentation", lastDocumentation());
    }
    if (!lastValidation().isNull())
    {
        events << Event("metadata", "lastvalidation", lastValidation());
    }
    // NOTE: further properties not handled

    foreach (const HistoryElement* e, d->history)
    {
        if (e->is<Therapy>())
        {
            const Therapy* t = static_cast<const Therapy*>(e);

            Event event("therapy", EventTherapyTypeTextIntMapper::toString(t->type), t->date);

            bool setCycles = false;

            if (!t->end.isNull())
            {
                event.infos << EventInfo("therapy:end", t->end.toString(Qt::ISODate));
            }
            if (t->outcome != Finding::UndefinedResult)
            {
                event.infos << EventInfo("therapy:outcome", EventFindingResultTextIntMapper::toString(t->outcome));
            }
            if (t->bestResponse != Finding::UndefinedResult)
            {
                event.infos << EventInfo("therapy:bestresponse", EventFindingResultTextIntMapper::toString(t->bestResponse));
            }
            if (!t->description.isEmpty())
            {
                event.infos << EventInfo("therapy:comment", t->description);
            }
            if (t->additionalInfos)
            {
                if (t->additionalInfos & Therapy::BeginsTherapyBlock)
                {
                    event.infos << EventInfo("therapy:block", "begin");
                }
                if (t->additionalInfos & Therapy::EndsTherapyBlock) // no else
                {
                    event.infos << EventInfo("therapy:block", "end");
                }
            }

            foreach (TherapyElement* te, t->elements)
            {
                if (te->is<Chemotherapy>())
                {
                    const Chemotherapy* ctx = static_cast<const Chemotherapy*>(te);
                    QMap<QString, QString> map;
                    map["main"] = ctx->substance;
                    if (ctx->dose)
                    {
                        map["dose"] = fromNumber(ctx->dose);
                    }
                    if (ctx->absdose)
                    {
                        map["absdose"] = fromNumber(ctx->absdose);
                    }
                    if (!ctx->schedule.isEmpty())
                    {
                        map["schedule"] = ctx->schedule;
                    }
                    event.infos << EventInfo("therapy:ctx:substance", mergeWithColonFields(map));

                    if (ctx->cycles && ! setCycles)
                    {
                        event.infos << EventInfo("therapy:ctx:cycles", QString().setNum(ctx->cycles));
                    }
                }
                else if (te->is<Radiotherapy>())
                {
                    const Radiotherapy* rtx = static_cast<const Radiotherapy*>(te);
                    QMap<QString, QString> map;
                    map["main"] = rtx->location;
                    if (rtx->dose)
                    {
                        map["dose"] = fromNumber(rtx->dose);
                    }
                    event.infos << EventInfo("therapy:rtx:treatment", mergeWithColonFields(map));
                }
                else if (te->is<Toxicity>())
                {
                    const Toxicity* tox = static_cast<const Toxicity*>(te);
                    QMap<QString, QString> map;
                    map["main"] = tox->description;
                    if (tox->grade)
                    {
                        map["grade"] = fromNumber(tox->grade);
                    }
                    event.infos << EventInfo("therapy:toxicity", mergeWithColonFields(map));
                }
            }
            d->addUnknownEventInfos(t, event);

            events << event;
        }
        else if (e->is<Finding>())
        {
            const Finding* f = static_cast<const Finding*>(e);
            Event event("finding", EventFindingTypeTextIntMapper::toString(f->type), f->date);
            if (f->result != Finding::UndefinedResult)
            {
                event.infos << EventInfo("finding:assessment", EventFindingResultTextIntMapper::toString(f->result));
            }
            if (!f->description.isEmpty())
            {
                event.infos << EventInfo("finding:comment", f->description);
            }
            if (f->context != Finding::UndefinedContext)
            {
                event.infos << EventInfo("finding:context", EventFindingContextTextIntMapper::toString(f->context));
            }
            if (f->additionalInfos & Finding::CentralNervous)
            {
                event.infos << EventInfo("finding:site", "cns");
            }
            if (f->additionalInfos & Finding::Metastasis)
            {
                event.infos << EventInfo("finding:event", "firstmetastasis");
            }
            if (f->additionalInfos & Finding::LocalRecurrence)
            {
                event.infos << EventInfo("finding:event", "localrelapse");
            }
            if (f->modality != Finding::UndefinedModality)
            {
                event.infos << EventInfo("finding:imaging:modality", EventFindingModalityTextIntMapper::toString(f->modality));
            }
            d->addUnknownEventInfos(f, event);

            events << event;
        }
        else if (e->is<DiseaseState>())
        {
            const DiseaseState* s = static_cast<const DiseaseState*>(e);
            Event event("diseasestate", EventDiseaseStateStateTextIntMapper::toString(s->state), s->date);
            d->addUnknownEventInfos(s, event);
            events << event;
        }
    }

    events += d->unknownEvents;

    return events;
}

//*

#include "diseasehistorymodel.h"
#include <QTreeView>

void DiseaseHistory::test()
{
    DiseaseHistory h;
    Finding* f1 = new Finding;
    f1->context = Finding::Antecedent;
    f1->date = QDate(2012, 8, 1);
    f1->type = Finding::Clinical;
    f1->description = "pain";
    h << f1;

    Finding* f2 = new Finding;
    f2->context = Finding::InitialDiagnosis;
    f2->date = QDate(2012, 9, 15);
    f2->type = Finding::Imaging;
    f2->modality = Finding::CT;
    h << f2;

    Therapy* t = new Therapy;
    t->date = QDate(2012, 10, 15);
    t->end = QDate(2012, 12, 24);
    t->description = "Cisplatin / Paclitaxel";
    t->type = Therapy::CTx;
    h << t;

    Chemotherapy* ctx1 = new Chemotherapy;
    ctx1->substance = "cisplatin";
    ctx1->dose = 100;
    ctx1->schedule = "d1+8 q21d";
    t->elements << ctx1;

    Chemotherapy* ctx2 = new Chemotherapy;
    ctx2->substance = "paclitaxel";
    ctx2->dose = 175;
    ctx2->schedule = "d1 q21d";
    t->elements << ctx2;

    Finding* f3 = new Finding;
    f3->context = Finding::ResponseEvaluation;
    f3->date = QDate(2013, 01, 15);
    f3->type = Finding::Imaging;
    f3->modality = Finding::MRI;
    f3->result = Finding::ProgressiveDisease;
    h << f3;

    Finding* f4 = new Finding;
    f4->context = Finding::ResponseEvaluation;
    f4->date = QDate(2013, 01, 15);
    f4->type = Finding::Clinical;
    f4->result = Finding::ProgressiveDisease;
    h << f4;

    Finding* f5 = new Finding;
    f5->date = QDate(2013, 02, 15);
    f5->type = Finding::Death;
    h << f5;

    DiseaseState* ds = new DiseaseState;
    ds->date = h.end();
    ds->state = DiseaseState::Deceased;
    h << ds;

    QList<Event> events = h.toEvents();
    DiseaseHistory h2 = DiseaseHistory::fromEvents(events);
    qDebug() << "Equal:" << (h == h2);
    return;

    DiseaseHistoryModel* model = new DiseaseHistoryModel;
    model->setHistory(h);
    QTreeView* tv = new QTreeView;
    tv->setModel(model);
    tv->show();

    DiseaseHistoryModel* model2 = new DiseaseHistoryModel;
    model2->setHistory(h2);
    QTreeView* tv2 = new QTreeView;
    tv2->setModel(model2);
    tv2->show();
    /*QString xml = h.toXml();

    DiseaseHistory h2 = DiseaseHistory::fromXml(xml);
    QString xml2 = h2.toXml();
    qDebug() << "Differs" << bool(xml != xml2);
    qDebug() << xml;
    qDebug() << xml2;
    */
}

bool DiseaseHistory::testXmlEvent() const
{
    QList<Event> events = toEvents();
    DiseaseHistory h2 = DiseaseHistory::fromEvents(events);
    QString xml1 = toXml();
    QString xml2 = h2.toXml();
    if (xml1 != xml2)
    {
        QTextStream str(stdout);
        str << xml1;
        str << '\n';
        str << xml2;
        return false;
    }
    return true;
}

//*/

