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

// Qt includes

#include <QDebug>
#include <QPair>

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
    PropertyList properties;
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

HistoryElementList& DiseaseHistory::entries()
{
    return d->history;
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
        d->properties.removeProperty("lastDocumentation");
    }
    else
    {
        d->properties.setProperty("lastDocumentation", date.toString(Qt::ISODate));
    }
}

QDate DiseaseHistory::lastDocumentation() const
{
    Property prop = d->properties.property("lastDocumentation");
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
        d->properties.removeProperty("lastValidation");
    }
    else
    {
        d->properties.setProperty("lastValidation", date.toString(Qt::ISODate));
    }
}

QDate DiseaseHistory::lastValidation() const
{
    Property prop = d->properties.property("lastValidation");
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
    Pair("ct", Finding::CT),
    Pair("mri", Finding::MRI),
    Pair("xray", Finding::XRay),
    Pair("sono", Finding::Sono),
    Pair("pet-ct", Finding::PETCT),
    Pair("scintigraphy", Finding::Scintigraphy),
    Pair("death", Finding::Death)
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
    Pair("infoCentralNervös", Finding::CentralNervous),
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
            stream.readAttributeCheckedEnum<Finding::Type, FindingTypeTextIntMapper>("type", f->type);
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

// *

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
    qDebug() << "First finding" << f1 << f1->context << f1->date << f1->type << f1->description;
    h << f1;
    Finding* f2 = new Finding;
    f2->context = Finding::InitialDiagnosis;
    f2->date = QDate(2012, 9, 15);
    f2->type = Finding::CT;
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
    f3->type = Finding::CT;
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

    DiseaseHistoryModel* model = new DiseaseHistoryModel;
    model->setHistory(h);
    QTreeView* tv = new QTreeView;
    tv->setModel(model);
    tv->show();

    /*QString xml = h.toXml();

    DiseaseHistory h2 = DiseaseHistory::fromXml(xml);
    QString xml2 = h2.toXml();
    qDebug() << "Differs" << bool(xml != xml2);
    qDebug() << xml;
    qDebug() << xml2;
    */
}
//*/

