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

class VisualHistoryWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VisualHistoryWidget(QWidget *parent = 0);
    ~VisualHistoryWidget();

    virtual QSize sizeHint() const;
    
    static QColor colorForState(DiseaseState::State state);
    static QColor colorForResult(Finding::Result result);

signals:
    
public slots:

    void setHistory(const DiseaseHistory& history);
    void setPixelsPerYear(int pixelsPerYear);

protected:

    virtual void paintEvent(QPaintEvent *event);

private:

    friend class StateColorDrawer;
    class VisualHistoryWidgetPriv;
    VisualHistoryWidgetPriv* const d;
    
};

#endif // VISUALHISTORYWIDGET_H
