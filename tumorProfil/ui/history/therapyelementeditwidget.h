/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 23.05.2013
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

#ifndef THERAPYELEMENTEDITWIDGET_H
#define THERAPYELEMENTEDITWIDGET_H

#include "historyelements.h"

#include <QWidget>

class QButtonGroup;
class QComboBox;
class QFormLayout;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QVBoxLayout;

class TherapyElementEditWidget : public QWidget
{
    Q_OBJECT
public:

    explicit TherapyElementEditWidget(QWidget *parent = 0);
    ~TherapyElementEditWidget();

    static TherapyElementEditWidget* create(TherapyElement* elem);
    
    virtual TherapyElement* applyToElement() const;
    TherapyElement* element() const;

    virtual void setReadOnly(bool readOnly);

Q_SIGNALS:

    void changed();
    void remove();

public slots:

    virtual void setElement(TherapyElement* element);

signals:
    
protected:

    TherapyElement* m_element;

    QLabel      *therapyLabel;
    QPushButton *clearButton;
    QVBoxLayout *therapyLayout;
};

class CTxEditWidget : public TherapyElementEditWidget
{
    Q_OBJECT

public:

    CTxEditWidget(QWidget* parent = 0);

    virtual TherapyElement* applyToElement() const;
    virtual void setReadOnly(bool readOnly);

public slots:

    virtual void setElement(TherapyElement* element);

private:

    QComboBox*    substanceBox;
    QLineEdit*    doseNumber;
    QButtonGroup* relAbsGroup;
    QLineEdit*    scheduleLineEdit;
};

class RTxEditWidget : public TherapyElementEditWidget
{
    Q_OBJECT

public:

    RTxEditWidget(QWidget* parent = 0);

    virtual TherapyElement* applyToElement() const;
    virtual void setReadOnly(bool readOnly);

public slots:

    virtual void setElement(TherapyElement* element);

private:

    QLineEdit*    doseNumberEdit;
    QLineEdit*    regionLineEdit;
};

class ToxicityEditWidget : public TherapyElementEditWidget
{
    Q_OBJECT

public:

    ToxicityEditWidget(QWidget* parent = 0);

    virtual TherapyElement* applyToElement() const;
    virtual void setReadOnly(bool readOnly);

public slots:

    virtual void setElement(TherapyElement* element);

private:

    QLineEdit*    gradeNumberEdit;
    QLineEdit*    typeLineEdit;
};

#endif // THERAPYELEMENTEDITWIDGET_H
