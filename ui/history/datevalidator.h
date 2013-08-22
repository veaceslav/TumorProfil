/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 10.06.2013
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

#ifndef DATEVALIDATOR_H
#define DATEVALIDATOR_H

#include <QValidator>
#include <QLineEdit>
#include <QDateTime>
#include <QKeyEvent>

const QDate MIN_DATE = QDate(1800,1,1);
const QDate MAX_DATE = QDate(4000,1,1);

QString DateToText(QDate date);
QDate TextToDate(QString text,QChar trennzeichen);

class DateValidator: public QValidator
{
    friend class Kalender;

    Q_OBJECT

    // Konstruktor und Destruktor

    public:
        DateValidator( QWidget * parent );
        ~DateValidator();

    // statische Umrechenmethoden

    public:
        static bool		IsValid(const QString &text);
        static void     setDate(QLineEdit* lineEdit, const QDate& date);
        static QString DateToText(QDate date);
        static QDate TextToDate(QString text, QChar trennzeichen = '.');


    signals:

        void dateChanged(const QDate&);

    // Schnittstelle

    public:

        void setRange(QDate minDate, QDate maxDate);
        void setLineEdit(QLineEdit *pEditLine);
        void setJahrhunderGrenze(int grenze);
        void setTrennzeichen(char trennzeichen);
        void setKeinJahrEnabled(bool enable);
        void setCurrentDate(const QDate& date);

    // virtuelle Methoden zum Prüfen und korrigieren

    protected:

        virtual QValidator::State validate( QString &, int & ) const;
        virtual void fixup ( QString & input ) const ;

    // Ereignisfilter für das überwachte Editfeld

    protected:
        virtual bool eventFilter ( QObject *, QEvent * ) ;

    protected :

        void setValidate(bool on);

    private :
        QDate m_minDate;
        QDate m_maxDate;
        QDate m_currentDate;

        QChar m_trennzeichen;
        int   m_jahrhundertGrenze;
        bool  m_keinJahrEnabled;
        bool  m_eventFilterOn;
        bool  m_lastRelease;

        static bool m_noValidate;
};


#endif // DATEVALIDATOR_H
