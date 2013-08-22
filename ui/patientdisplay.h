/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 30.01.2012
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

#ifndef PATIENTDISPLAY_H
#define PATIENTDISPLAY_H

// Qt includes

#include <QWidget>

// Local includes

#include "patient.h"

class PatientDisplay : public QWidget
{
    Q_OBJECT
public:
    explicit PatientDisplay(QWidget *parent = 0);
    ~PatientDisplay();

    void setShowGender(bool show);

public slots:

    void setPatient(const Patient::Ptr& patient);
    void clear();

private:

    class PatientDisplayPriv;
    PatientDisplayPriv* const d;
};

#endif // PATIENTDISPLAY_H
