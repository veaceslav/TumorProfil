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

#ifndef PATIENTENTERFORM_H
#define PATIENTENTERFORM_H

#include <QWidget>

// Local includes

#include "patient.h"

class PatientEnterForm : public QWidget
{
    Q_OBJECT
public:
    explicit PatientEnterForm(QWidget *parent = 0);
    ~PatientEnterForm();

    Patient currentPatient() const;
    /// Returns true if the current data is valid
    bool    isValid() const;
    
signals:

    void editingFinished(const Patient& currentValue);
    
public slots:

    void newPatient();
    void setValues(const Patient& patient);

protected slots:

    void returnPressed();

protected:

    virtual void keyPressEvent(QKeyEvent* e);

private:

    class PatientEnterFormPriv;
    PatientEnterFormPriv* const d;
    
};

#endif // PATIENTENTERFORM_H
