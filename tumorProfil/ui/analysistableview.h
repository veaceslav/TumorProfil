/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 20.05.2012
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

#ifndef ANALYSISTABLEVIEW_H
#define ANALYSISTABLEVIEW_H

#include <QTableView>

class AnalysisTableView : public QTableView
{
    Q_OBJECT

public:

    explicit AnalysisTableView(QWidget *parent = 0);
    
public slots:

    void copy();
    void toCSV();

protected:

    QString toCSV(const QList<bool>& columns) const;

    void keyPressEvent(QKeyEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void installDelegateToDisplayPercentages();

    virtual void addContextMenuActions(QMenu* menu);
};

#endif // ANALYSISTABLEVIEW_H
