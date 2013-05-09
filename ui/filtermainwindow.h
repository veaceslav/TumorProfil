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

#ifndef FILTERMAINWINDOW_H
#define FILTERMAINWINDOW_H

// Qt includes

#include <QMainWindow>

class PatientPropertyModelViewAdapter;

class FilterMainWindow : public QMainWindow
{
public:
    Q_OBJECT
public:

    explicit FilterMainWindow(QWidget *parent = 0);
    ~FilterMainWindow();

    PatientPropertyModelViewAdapter* adapter() const;
    /// Returns the toolbar created by setupToolbar
    QToolBar* toolBar() const;

protected slots:

    void entryActivated();
    void filterByContext();
    void filterByDate();
    void clearDateFilter();

protected:

    void setupToolbar();

    class FilterMainWindowPriv;
    FilterMainWindowPriv* const d;
};

#endif // FILTERMAINWINDOW_H
