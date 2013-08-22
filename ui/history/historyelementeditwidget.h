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

#ifndef HISTORYELEMENTEDITWIDGET_H
#define HISTORYELEMENTEDITWIDGET_H

// Qt includes

#include <QWidget>

// Local includes

#include "historyelements.h"

class QButtonGroup;
class QComboBox;
class QFormLayout;
class QHBoxLayout;
class QLineEdit;
class QPushButton;
class QVBoxLayout;

class DateCommentWidget : public QWidget
{
    Q_OBJECT
public:

    DateCommentWidget();

    enum Mode
    {
        DateRangeMode,
        SingleDateMode
    };

    void setMode(Mode mode);
    void setCommentEditVisible(bool visible);

    QDate date() const;
    QDate beginDate() const { return date(); }
    QDate endDate() const;
    QString comment() const;

    void setDate(const QDate& date);
    void setBeginDate(const QDate& date) { return setDate(date); }
    void setEndDate(const QDate& date);
    void setComment(const QString& comment);

Q_SIGNALS:

    void changed();

private:

    QFormLayout*    formLayout;
    QLineEdit*      firstDateEdit;
    QLineEdit*      endDateEdit;
    QLineEdit*      commentLineEdit;
};

class HistoryElementEditWidget : public QWidget
{
    Q_OBJECT

public:

    explicit HistoryElementEditWidget(QWidget *parent = 0);
    ~HistoryElementEditWidget();

    static HistoryElementEditWidget* create(HistoryElement* e);

    virtual HistoryElement* applyToElement() const;
    HistoryElement* element() const;
    virtual QString heading() const = 0;

Q_SIGNALS:

    void changed();

public slots:

    virtual void setElement(HistoryElement* element);

protected:

    HistoryElement*    m_element;
    DateCommentWidget* dateCommentWidget;
    QVBoxLayout*       mainLayout;
    
private:
};

class TherapyElementEditWidget;

class TherapyEditWidget : public HistoryElementEditWidget
{
    Q_OBJECT
public:

    TherapyEditWidget();

    virtual HistoryElement* applyToElement() const;
    virtual void setElement(HistoryElement* element);
    virtual QString heading() const;
    void removeElementUI(TherapyElement* te);

Q_SIGNALS:

    void addTherapyElement(TherapyElement* te);
    void therapyElementChanged(TherapyElement* te);
    void therapyElementRemove(TherapyElement* te);

protected:

    QList<TherapyElementEditWidget*> elementWidgets;
    void addElement(TherapyElement* te);
    void addElementUI(TherapyElement* te);

protected Q_SLOTS:

    void addSubstance();
    void addRadiation();
    void addToxicity();
    void slotTherapyElementChanged();
    void slotTherapyElementRemove();

Q_SIGNALS:

    void elementAdded(TherapyElement* element);

private:

    QVBoxLayout* elementWidgetLayout;
    QPushButton* moreCTxButton;
    QPushButton* moreRTxButton;
    QPushButton* moreToxButton;
};

class FindingEditWidget : public HistoryElementEditWidget
{
public:

    FindingEditWidget();

    virtual HistoryElement* applyToElement() const;
    virtual void setElement(HistoryElement* element);
    virtual QString heading() const { return tr("Befund"); }

private:

    QButtonGroup*       modalityGroup;
    QButtonGroup*       contextGroup;
    QButtonGroup*       resultGroup;
    QLineEdit*          commentLineEdit;
};

class DiseaseStateEditWidget : public HistoryElementEditWidget
{
public:

    DiseaseStateEditWidget();
    virtual QString heading() const { return tr("Status"); }

    virtual HistoryElement* applyToElement() const;
    virtual void setElement(HistoryElement* element);

protected:

    QButtonGroup*      statusGroup;
};


#endif // HISTORYELEMENTEDITWIDGET_H
