/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 22.02.2012
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

#ifndef REPORTWINDOW_H
#define REPORTWINDOW_H


// Local includes

#include "filtermainwindow.h"
#include "patient.h"
#include <QModelIndex>

class ReportTableView;

class ReportWindow : public FilterMainWindow
{
    Q_OBJECT
public:

    explicit ReportWindow(QWidget *parent = 0);
    ~ReportWindow();

    ReportTableView* view() const;

protected slots:

    void setAggregateVisible(bool);
    void activatedFromAggregate(const QList<QModelIndex>& sourceReferenceIndexes);

private:

    void setupToolbar();
    void setupView();

    class ReportWindowPriv;
    ReportWindowPriv* const d;
};

#endif // REPORTWINDOW_H
