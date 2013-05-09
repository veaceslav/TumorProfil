/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 28.02.2013
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

#ifndef HISTORYWINDOW_H
#define HISTORYWINDOW_H

#include <QMainWindow>

// Local includes

#include "filtermainwindow.h"

class HistoryWindow : public FilterMainWindow
{
    Q_OBJECT
public:

    explicit HistoryWindow(QWidget *parent = 0);
    ~HistoryWindow();

    //ReportTableView* view() const;

protected slots:

    void entryActivated();
private:

    void setupToolbar();
    void setupView();

    class Private;
    Private* const d;
};

#endif // HISTORYWINDOW_H
