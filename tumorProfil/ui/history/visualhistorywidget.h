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

#ifndef VISUALHISTORYWIDGET_H
#define VISUALHISTORYWIDGET_H

#include <QWidget>

// Local includes

#include "diseasehistory.h"
#include "history/historyiterator.h"

class VisualHistoryWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VisualHistoryWidget(QWidget *parent = 0);
    ~VisualHistoryWidget();

    virtual QSize sizeHint() const;

    void setProofReader(HistoryProofreader* pr);
    void updateLastDocumentation(const QDate& date);
    
    static QColor colorForState(DiseaseState::State state);
    static QColor colorForResult(Finding::Result result);

    QByteArray renderToSVG();
    QImage renderToImage();

signals:

    void clicked(const QDate& date);
    void clicked(HistoryElement* e);
    
public slots:

    void setHistory(const DiseaseHistory& history);
    void setCursor(const QDate& date);
    void setPixelsPerYear(int pixelsPerYear);
    void copy();

protected:

    virtual void paintEvent(QPaintEvent *event);
    void render(QPainter& p, bool widgetOutput = false);
    virtual void mousePressEvent(QMouseEvent* e);

private:

    friend class StateColorDrawer;
    class VisualHistoryWidgetPriv;
    VisualHistoryWidgetPriv* const d;
    
};

#endif // VISUALHISTORYWIDGET_H
