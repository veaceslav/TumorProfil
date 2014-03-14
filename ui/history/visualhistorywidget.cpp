/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 02.12.2013
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

#include "visualhistorywidget.h"

// Qt includes

#include <QtCore/qmath.h>
#include <QDebug>
#include <QPainter>

// Local includes

#include "history/historyiterator.h"

uint qHash(const QRect&r)
{
    return r.x() + 37*r.y() + 43*r.width() + 47*r.height();
}

class VisualHistoryWidget::VisualHistoryWidgetPriv
{
    friend class StateColorDrawer;
public:
    VisualHistoryWidgetPriv()
        : pixelsPerYear(100),
          height(60)
    {
    }

    DiseaseHistory history;
    int pixelsPerYear;
    int height;
    QList<QPair<QRect, HistoryElement*> > toolTipElements;

    int durationToPixels(const QDate& begin, const QDate& end) const
    {
        float days = qAbs(begin.daysTo(end));
        return qRound(days * pixelsPerYear / 365.0);
    }
};

QColor VisualHistoryWidget::colorForState(DiseaseState::State state)
{
    switch (state)
    {
    case DiseaseState::InitialDiagnosis:
        return Qt::cyan;
    case DiseaseState::Therapy:
        return Qt::darkYellow;
    case DiseaseState::BestSupportiveCare:
        return Qt::darkBlue;
    case DiseaseState::FollowUp:
        return Qt::white;
    case DiseaseState::Deceased:
        return Qt::black;
    case DiseaseState::LossOfContact:
        return Qt::blue;
    case DiseaseState::UnknownState:
        break;
    }
    return QColor();
}

QColor VisualHistoryWidget::colorForResult(Finding::Result result)
{
    switch (result)
    {
    case Finding::UndefinedResult:
    case Finding::ResultNotApplicable:
        break;
    case Finding::StableDisease:
        return Qt::darkYellow;
    case Finding::ProgressiveDisease:
        return Qt::red;
    case Finding::MinorResponse:
    case Finding::PartialResponse:
        return Qt::green;
    case Finding::CompleteResponse:
        return QColor(Qt::green).lighter();
    case Finding::NoEvidenceOfDisease:
        return Qt::white;
    case Finding::InitialFindingResult:
        return Qt::cyan;
    case Finding::Recurrence:
        return Qt::magenta;
    }
    return QColor();
}

VisualHistoryWidget::VisualHistoryWidget(QWidget *parent) :
    QWidget(parent),
    d(new VisualHistoryWidgetPriv)
{
    setAutoFillBackground(true);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

VisualHistoryWidget::~VisualHistoryWidget()
{
    delete d;
}

void VisualHistoryWidget::setHistory(const DiseaseHistory& history)
{
    d->history = history;
    d->toolTipElements.clear();
    //qDebug() << d->history.size() << isVisible() << "calling updateGeometry";
    updateGeometry();
    update();
}

void VisualHistoryWidget::setPixelsPerYear(int pixelsPerYear)
{
    d->pixelsPerYear = pixelsPerYear;
    if (!d->history.isEmpty())
    {
        updateGeometry();
        update();
    }
}

class StateColorDrawer
{
public:
    StateColorDrawer(QPainter* p, VisualHistoryWidget::VisualHistoryWidgetPriv* d,
                     const QDate& beginDate, int x, int y, int height)
        : p(p),
          x(x),
          height(height),
          y(y),
          lastState(DiseaseState::UnknownState),
          lastDate(beginDate),
          lastDefiningElement(0),
          d(d)
    {
    }
    QPainter* p;
    int x;
    const int height;
    const int y;
    DiseaseState::State lastState;
    QDate lastDate, lastLimitDate;
    HistoryElement* lastDefiningElement;
    VisualHistoryWidget::VisualHistoryWidgetPriv* const d;


    QString stateToText(DiseaseState::State state)
    {
    switch (state)
    {
    case DiseaseState::UnknownState:
        return "unbekannt";
    case DiseaseState::InitialDiagnosis:
        return "Erstdiagnose";
    case DiseaseState::Therapy:
        return "Therapie";
    case DiseaseState::BestSupportiveCare:
        return "Best Supportive Care";
    case DiseaseState::FollowUp:
        return "Nachsorge";
    case DiseaseState::Deceased:
        return "Verstorben";
    case DiseaseState::LossOfContact:
        return "Kontakt abgebrochen";
    default:
        return "?";
    }
    }


    // Cave: This is "retrospective", we end the paint operation for the previous state
    // only when we know the beginning of the next state
    void visit(DiseaseState::State currentState, HistoryElement* definingElement,
               const QDate& currentDate, const QDate& limitDate = QDate())
    {
        // First: draw state from last state till currentDate
        QDate endDate = currentDate;
        QDate nextLimitDate = limitDate;
        if (lastDate > currentDate)
        {
            qDebug() << "true conflict between states at" << currentDate << "and last state at" << lastDate;
        }
        if (lastLimitDate.isValid())
        {
            if (lastLimitDate > currentDate.addDays(1))
            {
                // endDate must be <= currentDate else the drawing will be off
                if (currentState == lastState)
                {
                    // keep end date at current date,
                    // and continue for next drawing operation with longer limit date
                    nextLimitDate = qMax(lastLimitDate, limitDate);
                }
                else
                {
                    qDebug() <<"conflict between states at" << currentDate << "last state valid to" << lastLimitDate
                             << "skipping its last part, please check";
                    // keep end date at current date
                }
            }
            else if (lastLimitDate < currentDate.addDays(-1))
            {
                qDebug() << "blind dates between end of last state" << lastLimitDate << "and new state at" << currentDate;
                endDate = lastLimitDate;
            }
        }

        int pixels = d->durationToPixels(lastDate, endDate);
        qDebug() << "State" << stateToText(lastState)<< "from" << lastDate << "to" << endDate << "currentDate" << currentDate << "pixels" << pixels ;
        if (lastState != DiseaseState::UnknownState)
        {
            QColor c = VisualHistoryWidget::colorForState(lastState);
            p->setBrush(c);
            p->setPen(Qt::NoPen);
            QRect r;
            if (pixels == 0)
            {
                if (lastState == DiseaseState::Deceased)
                {
                    r = QRect(x, y - 3, 2, height + 6);
                }
                else
                {
                    r = QRect(x, y - 2, 1, height + 4);
                }
            }
            else
            {
                r = QRect(x, y, pixels, height);
            }
            p->drawRect(r);
            d->toolTipElements << qMakePair(r, definingElement);
        }
        x += pixels;

        // "grey" area if previous state was limited before this state began
        if (endDate != currentDate)
        {
            x += d->durationToPixels(endDate, currentDate);
        }

        lastState = currentState;
        lastDate  = currentDate;
        lastLimitDate = nextLimitDate;
        lastDefiningElement = definingElement;
    }
};

void VisualHistoryWidget::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPen normalPen = p.pen();

    //qDebug() << "Painting history with" << d->history.entries().size() << "elements";

    d->toolTipElements.clear();
    if (d->history.isEmpty())
    {
        return;
    }

    int currentY = 0;

    // Metrics
    const int margin       = 5;
    const int statusHeight = 15;

    currentY += margin;

    // Effective DiseaseState
    StateColorDrawer stateDrawer(&p, d, d->history.begin(), margin, currentY, statusHeight);
    for (EffectiveStateIterator effectiveState(d->history);
         effectiveState.next() == HistoryIterator::Match; )
    {
        stateDrawer.visit(effectiveState.effectiveState(),
                          effectiveState.definingElement(),
                          effectiveState.definingElement()->date,
                          effectiveState.stateValidTo());
    }
    stateDrawer.visit(DiseaseState::UnknownState, 0, d->history.end());
    currentY += statusHeight;

    currentY += margin;

    const int linesHeight = 10;
    const int linesVerticalLimiterHeight = 6;

    NewTreatmentLineIterator treatmentLinesIterator;
    treatmentLinesIterator.set(d->history);
    treatmentLinesIterator.iterateToEnd();
    //qDebug() << "Have" << treatmentLinesIterator.therapies().size() << "therapies";
    int linesX = margin;
    QPen linesPen(Qt::black, 1.5);
    p.setPen(linesPen);
    QDate lastEndDate = d->history.begin();
    foreach (const TherapyGroup& group, treatmentLinesIterator.therapies())
    {
        linesX += d->durationToPixels(lastEndDate, group.beginDate());
        int pixels = d->durationToPixels(group.beginDate(), group.endDate());
        //qDebug() << "Group" << group.substances() <<group.beginDate() << group.endDate() << "pixels" << pixels;
        int limiterMargin = (linesHeight - linesVerticalLimiterHeight) / 2;
        p.drawLine(linesX, currentY + limiterMargin,
                   linesX, currentY + limiterMargin + linesVerticalLimiterHeight);
        p.drawLine(linesX + pixels, currentY + limiterMargin,
                   linesX + pixels, currentY + limiterMargin + linesVerticalLimiterHeight);
        int mainLineY = currentY + linesHeight/2;
        p.drawLine(linesX, mainLineY, linesX + pixels, mainLineY);
        foreach (HistoryElement* e, group)
        {
            d->toolTipElements << qMakePair(QRect(linesX, currentY, pixels, linesHeight), e);
        }

        linesX += pixels;
        lastEndDate = group.endDate();
    }
    currentY += linesHeight;
    p.setPen(normalPen);

    currentY += margin;

    const int radius = 2;
    foreach (const Finding* f, d->history.entries().filtered<Finding>())
    {
        switch (f->type)
        {
        case Finding::UndefinedType:
        case Finding::Histopathological:
        case Finding::Death:
            continue;
        case Finding::Clinical:
        case Finding::CT:
        case Finding::MRI:
        case Finding::XRay:
        case Finding::Sono:
        case Finding::PETCT:
        case Finding::Scintigraphy:
            break;
        }
        QDate begin = d->history.begin();
        int findingX = d->durationToPixels(begin, f->date) + margin;
        QColor c = colorForResult(f->result);
        p.setBrush(c);
        p.setPen(QPen(c, 0));
        p.drawChord(findingX-radius, currentY-radius, 2*radius, 2*radius, 0, 16*360);
    }
}

QSize VisualHistoryWidget::sizeHint() const
{
    if (d->history.isEmpty())
    {
        return QSize(0, d->height);
    }
    float days = d->history.begin().daysTo(d->history.end());
    //qDebug() << "sizeHint" << QSize(qCeil(days / 356)*d->pixelsPerYear, d->height) << "size" << size();
    return QSize(qCeil(days / 356)*d->pixelsPerYear, d->height);
}
