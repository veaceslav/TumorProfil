/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 2012-01-22
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>

// Local includes

#include "patient.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:

    void enterNewPatient();
    void newPatientEntered(const Patient& p);
    void setPatient(const Patient::Ptr& p);

    void save();
    void discardChanges();
    void showReport();

protected:

    void setupToolbar();
    void setupStatusBar();
    void setupUI();

    void closeEvent(QCloseEvent *);
    void keyPressEvent(QKeyEvent *);

protected slots:

    void patientNameEdited(const Patient& p);
    void patientNumberChanged();
    void selectFilteredPatient();

private:

    class MainWindowPriv;
    MainWindowPriv* const d;
};

#endif // MAINWINDOW_H
