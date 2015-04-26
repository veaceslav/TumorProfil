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
        : pixelsPerYear(200),
          height(60),
          proofreader(0)
    {
    }

    DiseaseHistory history;
    int pixelsPerYear;
    int height;
    QList<QPair<QRect, HistoryElement*> > toolTipElements;
    HistoryProofreader* proofreader;

    int durationToPixels(const QDate& begin, const QDate& end) const
    {
        float days = qAbs(begin.daysTo(end));
        return qRound(days * pixelsPerYear / 365.0);
    }

    void reportProblem(const HistoryElement* e, const QString& problem)
    {
        if (proofreader)
        {
            proofreader->problem(e, problem);
        }
        else
        {
            qDebug() << problem;
        }
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
    case DiseaseState::WatchAndWait:
        return Qt::lightGray;
    case DiseaseState::Deceased:
        return Qt::black;
    case DiseaseState::LossOfContact:
        return Qt::blue;
    case DiseaseState::UnknownState:
        return Qt::red;
        break;
    }
    return QColor();
}

QColor VisualHistoryWidget::colorForResult(Finding::Result result)
{
    switch (result)
    {
    case Finding::UndefinedResult:
        break;
    case Finding::ResultNotApplicable:
        return Qt::lightGray;
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

void VisualHistoryWidget::updateLastDocumentation(const QDate& date)
{
    DiseaseHistory history = d->history;
    history.setLastDocumentation(date);
    setHistory(history);
}

void VisualHistoryWidget::setProofReader(HistoryProofreader* pr)
{
    d->proofreader = pr;
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
    case DiseaseState::WatchAndWait:
        return "Verlaufskontrolle";
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


    void endVisit(const DiseaseHistory& history)
    {
        /*QDate endDate = qMax(history.end(), lastDate);
        endDate = qMax(endDate, lastLimitDate);
        switch (lastState)
        {
        case DiseaseState::BestSupportiveCare:
        case DiseaseState::WatchAndWait:
        case DiseaseState::FollowUp:
            endDate = qMax(endDate, history.lastDocumentation());
        default:
            break;
        }*/
        CurrentStateIterator it(history);
        //qDebug() << "endVisit" << lastState << lastDate << lastLimitDate << lastDefiningElement << it.effectiveHistoryEnd();
        visit(DiseaseState::UnknownState, 0, it.effectiveHistoryEnd());
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
            d->reportProblem(definingElement,
                             QString("true conflict between states at ")
                             + currentDate.toString()
                             + " and last state at "
                             + lastDate.toString());
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
                    d->reportProblem(definingElement,
                                     "conflict between states at "
                                     + currentDate.toString()
                                     + " last state valid to "
                                     + lastLimitDate.toString()
                                     + " skipping its last part, please check");
                    // keep end date at current date
                }
            }
            else if (lastLimitDate < currentDate.addDays(-1))
            {
                d->reportProblem(definingElement,
                                 "blind dates between end of last state"
                                 + lastLimitDate.toString()
                                 + "and new state at"
                                 + currentDate.toString());
                endDate = lastLimitDate;
            }
        }

        int pixels = d->durationToPixels(lastDate, endDate);
        //qDebug() << "State" << stateToText(lastState)<< "from" << lastDate << "to" << endDate << "currentDate" << currentDate << "pixels" << pixels ;
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

void VisualHistoryWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPen normalPen = p.pen();

    //qDebug() << "Painting history with" << d->history.entries().size() << "elements";

    d->toolTipElements.clear();
    if (d->proofreader)
    {
        d->proofreader->reset();
    }
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
    EffectiveStateIterator effectiveState(d->history);
    effectiveState.setProofreader(d->proofreader);
    for (; effectiveState.next() == HistoryIterator::Match; )
    {
        stateDrawer.visit(effectiveState.effectiveState(),
                          effectiveState.definingElement(),
                          effectiveState.definingElement()->date,
                          effectiveState.stateValidTo());
    }
    stateDrawer.endVisit(d->history);
    currentY += statusHeight;

    currentY += margin;

    const int linesHeight = 10;
    const int linesVerticalLimiterHeight = 6;

    NewTreatmentLineIterator treatmentLinesIterator;
    treatmentLinesIterator.setProofreader(d->proofreader);
    treatmentLinesIterator.set(d->history);
    treatmentLinesIterator.iterateToEnd();
    //qDebug() << "Have" << treatmentLinesIterator.therapies().size() << "therapies";
    int linesX = margin;
    QPen linesPen(Qt::black, 1.5);
    p.setPen(linesPen);
    QDate lastEndDate = d->history.begin();
    int count = 1;
    foreach (const TherapyGroup& group, treatmentLinesIterator.therapies())
    {
        if (group.effectiveEndDate() <= lastEndDate && lastEndDate != d->history.begin())
        {
            qDebug() << "Group" << group.substances() << group.beginDate() << group.endDate()
                     << "is contained in previous group";
            continue;
        }
        linesX += d->durationToPixels(lastEndDate, group.beginDate());
        int pixels = d->durationToPixels(qMax(lastEndDate, group.beginDate()), group.effectiveEndDate());
        //qDebug() << "Group" << group.substances() <<group.beginDate() << group.effectiveEndDate() << "pixels" << pixels;
        int limiterMargin = (linesHeight - linesVerticalLimiterHeight) / 2;
        // TTF line
        if (count == 2)
        {
            p.setPen(Qt::green);
            p.drawLine(linesX, currentY-10, linesX, currentY+linesHeight+10);
            p.setPen(linesPen);
        }

        // begin vertical line
        p.drawLine(linesX, currentY + limiterMargin,
                   linesX, currentY + limiterMargin + linesVerticalLimiterHeight);
        // end vertical line
        p.drawLine(linesX + pixels, currentY + limiterMargin,
                   linesX + pixels, currentY + limiterMargin + linesVerticalLimiterHeight);
        // horizontal line
        int mainLineY = currentY + linesHeight/2;
        p.drawLine(linesX, mainLineY, linesX + pixels, mainLineY);
        foreach (HistoryElement* e, group)
        {
            d->toolTipElements << qMakePair(QRect(linesX, currentY, pixels, linesHeight), e);
        }

        linesX += pixels;
        lastEndDate = group.effectiveEndDate();
        count++;
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
        case Finding::Death:
            continue;
        case Finding::Clinical:
        case Finding::CT:
        case Finding::MRI:
        case Finding::XRay:
        case Finding::Sono:
        case Finding::PETCT:
        case Finding::Scintigraphy:
        case Finding::Histopathological:
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
    CurrentStateIterator it(d->history);
    float days = d->history.begin().daysTo(it.effectiveHistoryEnd());
    //qDebug() << "sizeHint" << QSize(qCeil(days / 356)*d->pixelsPerYear, d->height) << "size" << size();
    return QSize(qCeil(days / 356)*d->pixelsPerYear, d->height);
}
