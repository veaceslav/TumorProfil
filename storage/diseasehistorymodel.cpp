/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 23.03.2013
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

#include "diseasehistorymodel.h"

// Qt includes

#include <QDebug>

namespace
{
enum Columns
{
    ColumnDescription,
    ColumnDate,
    ColumnInfo,
    ColumnCount,
    LastColumn = ColumnInfo
};
}

static inline HistoryElement* asHistoryElement(const QModelIndex& index)
{
    return static_cast<HistoryElement*>(index.internalPointer());
}

DiseaseHistoryModel::DiseaseHistoryModel(QObject *parent) :
    QAbstractItemModel(parent)
{
}

DiseaseHistory DiseaseHistoryModel::history() const
{
    return m_history;
}

void DiseaseHistoryModel::setHistory(const DiseaseHistory& history)
{
    beginResetModel();
    m_history = history;
    endResetModel();
}

QModelIndex DiseaseHistoryModel::index(HistoryElement* e, int col) const
{
    // only two-level hierarchy supported
    if (e->parent())
    {
        if (e->parent()->is<Therapy>() && e->is<TherapyElement>())
        {
            Therapy* t = e->parent()->as<Therapy>();
            TherapyElement* te = e->as<TherapyElement>();
            int index = t->elements.indexOf(te);
            if (index != -1)
            {
                return createIndex(index, col, e);
            }
        }
    }
    else
    {
        return createIndex(m_history.entries().indexOf(e), col, e);
    }
    return QModelIndex();
}

HistoryElement* DiseaseHistoryModel::element(const QModelIndex& index) const
{
    return asHistoryElement(index);
}

HistoryElement* DiseaseHistoryModel::retrieveElement(const QModelIndex& index)
{
    return index.data(HistoryElementRole).value<HistoryElement*>();
}

void DiseaseHistoryModel::addElement(HistoryElement* e)
{
    beginInsertRows(QModelIndex(), m_history.size(), m_history.size());
    m_history.entries() << e;
    endInsertRows();
}

void DiseaseHistoryModel::addElement(HistoryElement* parent, HistoryElement* e)
{
    QModelIndex parentIndex = index(parent);
    if (!parentIndex.isValid())
    {
        qDebug() << "Invalid insertion on unknown parent" << parent;
        return;
    }
    if (parent->is<Therapy>() && e->is<TherapyElement>())
    {
        qDebug() << "Adding therapy element";
        Therapy* t = parent->as<Therapy>();
        TherapyElement* te = e->as<TherapyElement>();
        beginInsertRows(parentIndex, t->elements.size(), t->elements.size());
        t->elements << te;
        endInsertRows();
    }
    else
    {
        qDebug() << "Children only supported for Therapy";
        return;
    }
}

HistoryElement* DiseaseHistoryModel::takeElement(HistoryElement* e)
{
    QModelIndex toRemove = index(e);
    if (!toRemove.isValid())
    {
        qDebug() << "Element" << e << "is not known, cannot remove";
        return 0;
    }
    QModelIndex p = parent(toRemove);
    beginRemoveRows(e->parent() ? parent(toRemove) : QModelIndex(), toRemove.row(), toRemove.row());
    m_history.remove(e);
    endRemoveRows();
    return e;
}

void DiseaseHistoryModel::elementChanged(HistoryElement* e)
{
    QModelIndex idx = index(e);
    if (!idx.isValid())
    {
        qDebug() << "Element" << e << "is not known, cannot react to changes";
        return;
    }
    emit dataChanged(idx, index(e, LastColumn));
}

QVariant DiseaseHistoryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (role != Qt::DisplayRole && role != HistoryElementRole)
    {
        return QVariant();
    }

    HistoryElement* e = asHistoryElement(index);

    if (role == HistoryElementRole)
    {
        return QVariant::fromValue<HistoryElement*>(e);
    }

    if (e->is<Therapy>())
    {
        const Therapy* t = e->as<Therapy>();

        switch (index.column())
        {
        case ColumnDescription:
            return Therapy::uiLabel(t->type);
        case ColumnDate:
            if (t->begin().isValid()) return t->begin(); else return tr("<Datum fehlt>");
        case ColumnInfo:
            switch (t->type)
            {
            case Therapy::CTx:
            case Therapy::RCTx:
            case Therapy::RTx:
                if (t->end.isValid()) return t->end; else return tr("<Datum fehlt>");
            case Therapy::Surgery:
            case Therapy::Intervention:
                return t->description;
                return QString();
            }
        }
    }
    else if (e->is<Chemotherapy>())
    {
        const Chemotherapy* t = e->as<Chemotherapy>();
        switch (index.column())
        {
        case ColumnDescription:
            return t->substance.isEmpty() ? tr("<Chemo>") : t->substance;
        case ColumnInfo:
            if (t->dose)
            {
                return QString("%1 mg/m²").arg(t->dose);
            }
        default:
            break;
        }
    }
    else if (e->is<Radiotherapy>())
    {
        Radiotherapy* rt = e->as<Radiotherapy>();
        switch (index.column())
        {
        case ColumnDescription:
            return rt->location.isEmpty() ? tr("<Bestrahlung>") : rt->location;
        case ColumnInfo:
            return rt->dose;
        default:
            break;
        }
    }
    else if (e->is<Toxicity>())
    {
        Toxicity* t = e->as<Toxicity>();
        switch (index.column())
        {
        case ColumnDescription:
            return t->description.isEmpty() ? tr("<Toxizität>") : t->description;
        case ColumnInfo:
            if (t->grade)
            {
                return QString("%1°").arg(t->grade);
            }
        default:
            break;
        }
    }
    else if (e->is<Finding>())
    {
        Finding* f = e->as<Finding>();
        QString modality = tr("Befund");
        QString result;
        bool wantAnswer = false;
        switch (f->type)
        {
        case Finding::UndefinedType:
            break;
        case Finding::Clinical:
            modality = tr("Klinisch");
            wantAnswer = true;
            break;
        case Finding::Histopathological:
            modality = tr("Histologie");
            break;
        case Finding::CT:
            modality = tr("CT");
            wantAnswer = true;
            break;
        case Finding::MRI:
            modality = tr("MRT");
            wantAnswer = true;
            break;
        case Finding::XRay:
            modality = tr("Röntgen");
            wantAnswer = true;
            break;
        case Finding::Sono:
            modality = tr("Sonographie");
            wantAnswer = true;
            break;
        case Finding::PETCT:
            modality = tr("PET-CT");
            wantAnswer = true;
            break;
        case Finding::Scintigraphy:
            modality = tr("Szintigraphie");
            wantAnswer = true;
            break;
        case Finding::Death:
            modality = tr("Verstorben");
            break;
        };

        if (wantAnswer && f->result != Finding::UndefinedResult)
        {
            switch (f->result)
            {
            case Finding::UndefinedResult:
                break;
            case Finding::StableDisease:
                result = tr("Stabile Erkrankung");
                break;
            case Finding::ProgressiveDisease:
                result = tr("Progress");
                break;
            case Finding::MinorResponse:
                result = tr("Geringes Ansprechen");
                break;
            case Finding::PartialResponse:
                result = tr("Partielles Ansprechen");
                break;
            case Finding::CompleteResponse:
                result = tr("Komplettes Ansprechen");
                break;
            case Finding::NoEvidenceOfDisease:
                result = tr("Kein Erkrankungsnachweis");
                break;
            case Finding::InitialFindingResult:
                result = tr("Erstbefund");
                break;
            case Finding::Recurrence:
                result = tr("Rezidiv");
                break;
            case Finding::ResultNotApplicable:
                break;
            }
        }
        switch (index.column())
        {
        case ColumnDescription:
            return modality;
        case ColumnDate:
            return f->date;
        case ColumnInfo:
            if (wantAnswer)
            {
                return result.isEmpty() ? tr("<Ergebnis>") : result;
            }
            else
            {
                return f->description;
            }
        default:
            break;
        }
    }
    else if (e->is<DiseaseState>())
    {
        DiseaseState* s = e->as<DiseaseState>();
        switch (index.column())
        {
        case ColumnDescription:
            switch (s->state)
            {
            case DiseaseState::UnknownState:
                return tr("Status unbekannt");
            case DiseaseState::InitialDiagnosis:
                return tr("Status: Erstdiagnose");
            case DiseaseState::Therapy:
                return tr("Status: Therapie");
            case DiseaseState::BestSupportiveCare:
                return tr("Konzept: Best Supportive Care");
            case DiseaseState::FollowUp:
                return tr("Status: Nachsorge");
            case DiseaseState::Deceased:
                return tr("Status: Verstorben");
            case DiseaseState::LossOfContact:
                return tr("Status: Kontakt abgebrochen");
            case DiseaseState::WatchAndWait:
                return tr("Status: Verlaufskontrolle");
            default:
                return tr("<Krankheitsstatus>");
            }
        case ColumnDate:
            return s->date;
        }
    }

    switch (role)
    {
    case Qt::DisplayRole:
        break;
    //case VariantDataRole:
        //break;
    }
    return QVariant();
}

QVariant DiseaseHistoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation ==Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case ColumnDescription:
            return tr("Vorgang");
        case ColumnDate:
            return tr("Datum");
        case ColumnInfo:
            return tr("Info");
        default:
            break;
        }
    }
    return QVariant();
}

int DiseaseHistoryModel::rowCount(const QModelIndex& parent ) const
{
    if (parent.isValid())
    {
        HistoryElement* elem = asHistoryElement(parent);
        if (elem->is<Therapy>())
        {
            //qDebug() << "rowCount for" << elem << "returns" << elem->as<Therapy>()->elements.size();
            return elem->as<Therapy>()->elements.size();
        }
        else
        {
            return 0;
        }
    }
    return m_history.size();
}

int DiseaseHistoryModel::columnCount(const QModelIndex& parent) const
{
    return ColumnCount;
}

Qt::ItemFlags DiseaseHistoryModel::flags(const QModelIndex& index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

bool DiseaseHistoryModel::hasChildren(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        HistoryElement* elem = asHistoryElement(parent);
        //qDebug() << "hasChildren for" << elem << "returns" << (elem->is<Therapy>() && elem->as<Therapy>()->elements.size());
        return elem->is<Therapy>() && elem->as<Therapy>()->elements.size();
    }
    return true;
}

QModelIndex DiseaseHistoryModel::index(int row, int column, const QModelIndex& parent ) const
{
    if (row < 0 || column < 0 || column > ColumnCount)
    {
        return QModelIndex();
    }
    // only two-level hierarchy supported
    if (parent.isValid())
    {
        HistoryElement* elem = asHistoryElement(parent);
        if (elem->is<Therapy>())
        {
            const Therapy* t = elem->as<Therapy>();
            if (row < t->elements.size())
            {
                return createIndex(row, column, t->elements[row]);
            }
        }
        return QModelIndex();
    }
    else
    {
        // top level
        if (row >= m_history.size())
        {
            return QModelIndex();
        }
        return createIndex(row, column, (void*)m_history[row]);
    }
    return QModelIndex();
}

QModelIndex DiseaseHistoryModel::parent(const QModelIndex& index) const
{
    if (index.isValid())
    {
        HistoryElement* elem = asHistoryElement(index);
        if (elem->parent())
        {
            HistoryElement* parent = elem->parent();
            // no two-level hierarchy at the moment
            Q_ASSERT(parent->parent() == 0);
            int row = m_history.entries().indexOf(parent);
            Q_ASSERT(row != -1);
            return createIndex(row, 0, parent);
        }
    }
    return QModelIndex();
}
