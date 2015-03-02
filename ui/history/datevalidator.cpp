/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 10.06.2013
 *
 * Copyright (C) 20?? by Arne Stocker   <arne@bdeichmann.de>
 * Copyright (C) 2013 by Marcel Wiesweg <marcel dot wiesweg at uk-essen dot de>
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

#include "datevalidator.h"

/**                      this programm code may be used for free             *
**                      there ist no garantee for any usability             *
**                                                                          *
**                      feel free to put any suggestion to                  *
**                      arne@bdeichmann.de                                  *
**                                                          (Arne Stocker)  *
*****************************************************************************/

/****************************************************************************
*                                                                           *
* Eingabemöglichkeiten  : 010205, 01.0205, 01.02.05 01022005 01.022005      *
*                         01.02.2005                                        *
* Pfeil links rechts    : selektiert den Tag, Monat oder Jahrbereich        *
* Pfeil oben unten      : zählt je nach ausgewähltem Bereich die Tage,      *
*                         Monate oder Jahre vor oder zurück                 *
* Fixup                 : wird beim Verlassen des EditWidget durchgeführt   *
* Signal                : dateChanged(QDate&) wird bei fixup gesendet       *
*                                                                           *
* Trennzeichen          : können selbst gesetzt werden, default = '.'       *
* Jahrhungergrenze      : kann zwischen 0 ] grenze [ 100 gesetzt werden     *
*                         default ist 50                                    *
* zulässiger Bereich    : einstellbar von (1800,1,1) bis (4000,1,1)         *
* unendlich             : 1.1.4000 wird wie kein Datum o. unendlich be-     *
*                         handelt und entsprich einem leeren text           *
*****************************************************************************/

/****************************************************************************
**  sorry for comment in german                                             *
*****************************************************************************/

#include "datevalidator.h"
#include <QDebug>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qapplication.h>


// Konstanten

bool DateValidator::m_noValidate = false;

// Funktionen

/************************************************************************
**	Funktion :	DateToText								A.28.09.04	    *
**																		*
**  Aufgabe	:	hier nicht implementiert, weil ich eine andere Methode  *
**              über eine von QDate abgeleitete Klasse benutze          *
**	Annahme :	-														*
**	Eingabe :	QDate													*
**	Ausgabe :	-														*
**	return	:	Text leer                                               *
*************************************************************************/

QString DateValidator::DateToText(const QDate&)
{
    return "";                                  // muss sich jeder nach geschmack selbst schreiben ...
                                                // oder die Methode von QDate nehmen
}

/************************************************************************
**	Funktion :	TextToDate								A.28.09.04	    *
**																		*
**  Aufgabe	:	Wandelt Text in QDate um. Bei fehlschlag wird           *
**              QDate(0,0,0) zurückgegeben                              *
**	Annahme :	    													*
**	Eingabe :	text													*
**	Ausgabe :	Int64 												    *
**	return	:	true oder false                                         *
*************************************************************************/

QDate DateValidator::TextToDate(QString input,QChar trennzeichen)
{
    int tagTrenner = -1;
    int monatTrenner = -1;
    int zuvielTrenner = -1;

    QString tag;
    QString monat;
    QString jahr;

    // prüfen, ob leertext zulässig

    if (input.isEmpty() == true)
    {
       return QDate();
    }

    // Alles raus ausser punkten und ziffern

    input = input.replace(QRegExp("[^0-9.]"),"");

    // Alle doppelten Punkte zu einem Punkt wandeln

    while (input.indexOf("..") > 0)
    {
        input = input.replace(QRegExp(".."),".");
    }

    // Zwei Punkte hintereinander -> auch raus

    tagTrenner    = input.indexOf(trennzeichen);
    monatTrenner  = input.indexOf(trennzeichen,tagTrenner + 1);
    zuvielTrenner = input.indexOf(trennzeichen,monatTrenner + 1);

    // tag und monat nur dann getrennt auslesen, wenn die
    // punkte nicht völlig verkrautet sind

    if (    (zuvielTrenner < 0)                                     &&
            ((   (tagTrenner == 1)  &&
                ( (monatTrenner == 3) || (monatTrenner == 4) )  )   ||
            (   (tagTrenner == 2)  &&
                ( (monatTrenner == 4) || (monatTrenner == 5) )  ))   )
    {
        tag   = input.left(tagTrenner);
        monat = input.mid(tagTrenner + 1,monatTrenner - tagTrenner - 1);
        jahr  = input.mid(monatTrenner +1);
    }

    // Die Punkte sind auch verkorkst also raus

    else
    {
        input = input.replace(QRegExp("[^0-9]"),"");
        tag  = input.left(2);
        monat = input.mid(2,2);
        jahr = input.mid(4,4);
    }



    bool tagOK   = false;
    bool monatOK = false;
    bool jahrOK  = false;

    int tagInt = tag.toInt(&tagOK);
    int monatInt = monat.toInt(&monatOK);
    int jahrInt = jahr.toInt(&jahrOK);

    return QDate(jahrInt,monatInt,tagInt);
}

void     DateValidator::setDate(QLineEdit* lineEdit, const QDate& date)
{
    if (date.isValid())
    {
        lineEdit->setText(date.toString("dd.MM.yyyy"));
    }
    else
    {
        lineEdit->setText(QString());
    }
}

/************************************************************************
**															    		*
** Konstruktor und Destruktor 											*
**																		*
*************************************************************************/

/************************************************************************
**	Funktion :	Konstruktor 								A.28.09.04	*
**																		*
**  Aufgabe	:	Konstruktor von DateValidator, setzt den minimal und    *
**              den maximal zulässigen Eurowert                         *
**	Annahme :	    													*
**	Eingabe :	parent, name    										*
**	Ausgabe :													        *
**	return	:	                                                        *
*************************************************************************/

DateValidator::DateValidator( QWidget* pParent ) : QValidator( pParent )
{
    // Die Minimal- und die Maximalwerte setzen
    m_minDate = QDate(1800,1,1);
    m_maxDate = QDate(3999,12,31);

    m_trennzeichen = '.';
    m_jahrhundertGrenze = 50;
    m_keinJahrEnabled = false;
    m_eventFilterOn = false;
    m_lastRelease = false;

    m_currentDate = QDate::currentDate();
}


DateValidator::~DateValidator()
{
    // das Reh springt hoch, das Reh springt weit
    // warum auch nicht, es hat ja Zeit
}

/************************************************************************
**															    		*
** Virtuelle Methoden zur Ãœberprüfung                                   *
**																		*
*************************************************************************/

/************************************************************************
** Methode :	SetLineEdit                                 A.28.09.04	*
**																		*
** Aufgabe :    Stellt sicher, dass ein EventFilter auf Objekt (nur	    *
**				QEditline und QComboBox) gesetzt wird und ruft an-      *
**				schlieÃŸend QEditLine->setValidator() auf. Zugleich	    *
**				wird ein erster Fixup durchgeführt. 				    *
**																		*
**	Annahme :	pEditLine ungleich Null									*
**	Eingabe :	QLineEdit, QComboBox                                    *
**	Ausgabe :													        *
**	return	:	                                                        *
*************************************************************************/

void DateValidator::setLineEdit(QLineEdit *pEditLine)
{
    Q_ASSERT(pEditLine != NULL);

    pEditLine->installEventFilter(this);        // Fixup bei Focus Left
    pEditLine->setValidator(0);
    pEditLine->setValidator(this);

    // richtigstellen

    QString currentText = pEditLine->text();
    fixup(currentText);
    setValidate(false);
    pEditLine->setText(currentText);
    setValidate(true);
}

/************************************************************************
** Methode :	SetRange     								A.28.09.04	*
**																		*
** Aufgabe :    Setzt den zulässigen Bereich der Eingabe (minimun und   *
**              und maximum                                             *
**																		*
**	Annahme :	pEditLine ungleich Null									*
**	Eingabe :	QLineEdit, QComboBox                                    *
**	Ausgabe :													        *
**	return	:	                                                        *
*************************************************************************/

void DateValidator::setRange(QDate minDate,QDate maxDate)
{
    // gesetzt wird der Wert als QDate
    if (minDate > MIN_DATE)
        minDate = MIN_DATE;

    if (maxDate > MAX_DATE)
        maxDate = MAX_DATE;

    m_minDate = minDate;
    m_maxDate = maxDate;
}


/************************************************************************
**  Methode :	SetJahrhunderGrenze 						A.28.09.04	*
**																		*
**  Aufgabe :   Setzt die Jahrhundergrenze, ab wann das Datum bei zwei- *
**              stelliger Eingabe aufgerunden (2000) und ab wann ab     *
**              gerundet (1900) wird                                    *
**              00 ist immer 2000                                       *
**                                                                      *
**              betrifft nur die Eingabe nicht die Darstellung          *
**                                                                      *
**	Annahme :	0 < trennzeichen < 100                                  *
**	Eingabe :	char                                                    *
**	Ausgabe :													        *
**	return	:	                                                        *
*************************************************************************/

void DateValidator::setJahrhunderGrenze(int grenze)
{
    m_jahrhundertGrenze = grenze;
}


/************************************************************************
**  Methode :	SetTrennzeichen								A.28.09.04	*
**																		*
**  Aufgabe :   Setzt das Trennzeichen 01.01.2000, 01-01-2000 usw       *
**	Annahme :	trennzeichen ungleich leerzeichen   					*
**	Eingabe :	char                                                    *
**	Ausgabe :													        *
**	return	:	                                                        *
*************************************************************************/

void DateValidator::setTrennzeichen(char trennzeichen)
{
    m_trennzeichen = trennzeichen;
}


/************************************************************************
**  Methode : validate     								    A.28.09.04	*
**																		*
**  Aufgabe : Prüft die Eingabe und gibt Invalid oder Intermediate      *
**            zurück. Ein Acceptable wird nicht vergeben (wie beim Jura *
**            Examen) weil zum Schluss immer noch die Punkte richtig    *
**            gesetzt erden müssen.										*
**																		*
**	Annahme :                           								*
**	Eingabe : input, pos                                                *
**	Ausgabe :													        *
**	return	: Intermediate, Invalid                                     *
**            (Acceptable nur wenn validate ausgeschaltet               *	                                                        *
*************************************************************************/


QValidator::State DateValidator::validate( QString & input, int & pos) const
{
    Q_UNUSED(pos)
    // nur validieren wenn nicht abgeschaltet

    if (m_noValidate == true)
        return Acceptable;

    QString tag;
    QString monat;
    QString jahr;

    // Die Trennzeichen werden entfernt

    // input = input.replace(QString(m_trennzeichen),"");

    // Es sind noch andere ungültige Ziffern vorhanden

    if (input.indexOf(QRegExp("[^0-9.]")) >= 0)
    {
        return QValidator::Invalid;
    }

    // mals sehen was die Punkte machen

    return QValidator::Intermediate;
}

/****************************************************************************
** Methode:     fixup                                            A.29.09.04	*
**                                                                          *
** Aufgabe :    Ãœbergeben wird eine grundsätzlich akzeptabel Eingabe (siehe *
**				Validate). Die Eingabe wird nur noch in die richtige Form	*
**				gebracht, d.h. mit tausender Punkten und mit zwei Stellen	*
**				nach dem Komma												*
**																			*
** Annahme :    übergebener Text ist intermediate                           *
** Eingabe :    eurotext                                                    *
** Rückgabe:    eurotext                                                    *
** return  :     -                                                          *
*****************************************************************************/

void DateValidator::fixup ( QString & input ) const
{
    int tagTrenner = -1;
    int monatTrenner = -1;
    int zuvielTrenner = -1;

    QString tag;
    QString monat;
    QString jahr;

    // prüfen, ob leertext zulässig

    if (    (input.isEmpty() == true)   &&
            (m_keinJahrEnabled == true) )
    {
        return;
    }

    // Alles raus ausser punkten und ziffern

    input = input.replace(QRegExp("[^0-9.]"),"");

    // Alle doppelten Punkte zu einem Punkt wandeln

    while (input.indexOf("..") > 0)
    {
        input = input.replace(QRegExp(".."),".");
    }

    // Zwei Punkte hintereinander -> auch raus

    tagTrenner    = input.indexOf(m_trennzeichen);
    monatTrenner  = input.indexOf(m_trennzeichen,tagTrenner + 1);
    zuvielTrenner = input.indexOf(m_trennzeichen,monatTrenner + 1);

    // tag und monat nur dann getrennt auslesen, wenn die
    // punkte nicht völlig verkrautet sind

    if (    (zuvielTrenner < 0)                                     &&
            ((   (tagTrenner == 1)  &&
                ( (monatTrenner == 3) || (monatTrenner == 4) )  )   ||
            (   (tagTrenner == 2)  &&
                ( (monatTrenner == 4) || (monatTrenner == 5) )  ))   )
    {
        tag   = input.left(tagTrenner);
        monat = input.mid(tagTrenner + 1,monatTrenner - tagTrenner - 1);
        jahr  = input.mid(monatTrenner +1);
    }

    // Die Punkte sind auch verkorkst also raus

    else
    {
        input = input.replace(QRegExp("[^0-9]"),"");
        tag  = input.left(2);
        monat = input.mid(2,2);
        jahr = input.mid(4,4);
    }

    QString ctag = tag;
    QString cmonat = monat;
    QString cjahr = jahr;


    bool tagOK   = false;
    bool monatOK = false;
    bool jahrOK  = false;

    int tagInt = tag.toInt(&tagOK);
    int monatInt = monat.toInt(&monatOK);
    int jahrInt = jahr.toInt(&jahrOK);

    if (tagInt <= 0)
        tagInt =  1;
    else if (tagInt > 31)
        tagInt = 31;

    // lastDayOfMonth nicht vergessen

    if (monatInt <= 0)
        monatInt = 1;
    else if (monatInt > 12)
        monatInt = 12;

    if (jahrInt < m_jahrhundertGrenze)
        jahrInt = 2000 + jahrInt;
    else if (jahrInt < 100)
        jahrInt = 1900 + jahrInt;
    else if (jahrInt < m_minDate.year())
        jahrInt = m_minDate.year() - 1;
    else if (jahrInt > m_maxDate.year())
        jahrInt = m_maxDate.year();

    QDate datum(jahrInt,monatInt,1);
    if (tagInt > datum.daysInMonth())
        tagInt = datum.daysInMonth();

    datum = QDate(jahrInt,monatInt,tagInt);

    if (datum < m_minDate)
    {
        tagInt = m_minDate.day();
        monatInt = m_minDate.month();
        jahrInt = m_minDate.year();
    }
    else if (datum > m_maxDate)
    {
        tagInt = m_maxDate.day();
        monatInt = m_maxDate.month();
        jahrInt = m_maxDate.year();
    }

    // Falls eine Umwandlung nicht OK war, ist das Datum zwar gültig
    // es wird jedoch das currentDatum gesetzt

    if (    (tagOK && monatOK && jahrOK) == false)
    {
        tagInt = m_currentDate.day();
        monatInt = m_currentDate.month();
        jahrInt = m_currentDate.year();
    }

    input = QString::number(tagInt).rightJustified(2,'0',true) + m_trennzeichen +
            QString::number(monatInt).rightJustified(2,'0',true) + m_trennzeichen +
            QString::number(jahrInt);
}

/****************************************************************************
**																			*
** eventFilter(..)	Sorgt dafür, dass beim Verlassen des Eingbe Controls	*
**					ein Key - Press Event übermittelt wird. Das Key - Press *
**					event hat zur Folgte, dass Fixup aufgerufen wird und	*
**					der Inhalt der Eingabe überprüft wird					*
**																			*
*****************************************************************************/

/****************************************************************************
** Methode:     eventFilter                                     A.29.09.04	*
**                                                                          *
** Aufgabe:     Sorgt dafür, dass beim Verlassen des Eingbe Controls	    *
**				ein fixup durchgeführt und der Inhalt der Eingabe überprüft *
**              wird					                                    *
**																			*
** Annahme :    pObject und pEvent ungeleich NULL                           *
** Eingabe :    zeiger auf object und pEvent                                *
** Rückgabe:    eurotext                                                    *
** return  :     -                                                          *
*****************************************************************************/

bool DateValidator::eventFilter ( QObject * pObject, QEvent * pEvent)
{
    // Rekursion bei direktem Aufruf (siehe KeyPress) verhindern

    if (m_eventFilterOn == true)
        return false;


    if ((pObject == NULL) || (pEvent == NULL))
    {
        return false;
    }

    // Wenn ein FocusOut Event vorliegt wird ein fixup an dem Text
    // durchgeführt und der Text zurückgeschrieben. Auf diese Weise
    // sieht das Ergebnis nach dem Verlassen der EditLine immer korrekt aus

    if (pEvent->type() == QEvent::FocusOut)
    {
        QLineEdit *pLineEdit = qobject_cast<QLineEdit*>(pObject);
        if (pLineEdit)
        {
            QString fixText = pLineEdit->text();
            fixup(fixText);
            setValidate(false);
            pLineEdit->setText(fixText);
            setValidate(true);
            QDate newDate = TextToDate(fixText,m_trennzeichen);
            pLineEdit->setProperty("date", newDate);
            emit dateChanged(newDate);
        }
    }

    if (pEvent->type() == QEvent::FocusIn)
    {
        QLineEdit *pLineEdit = qobject_cast<QLineEdit*>(pObject);
        qDebug() << "focus in" << pLineEdit;
        if (pLineEdit)
        {
            pLineEdit->selectAll();
        }
    }
    else if (pEvent->type() == QEvent::KeyPress)
    {
        QLineEdit *pLineEdit = qobject_cast<QLineEdit*>(pObject);
        if (pLineEdit)
        {
            m_lastRelease = false;
            QKeyEvent *pKeyEvent = (QKeyEvent*)pEvent;

            // Punkte herausfinden

            int tagTrenner    = pLineEdit->text().indexOf(m_trennzeichen);
            int monatTrenner  = pLineEdit->text().indexOf(m_trennzeichen,tagTrenner + 1);
            int pos           = pLineEdit->cursorPosition();

            // Wenn Pos bereits das Ende ist, dann fixup

            if ( (tagTrenner < 0) || (monatTrenner < 0) )
            {
                if (    ( (pKeyEvent->key() == Qt::Key_Left) && (pos == 0) ) ||
                        ( (pKeyEvent->key() == Qt::Key_Right)&& (pos == pLineEdit->text().length()) ) ||
                        (pKeyEvent->key() == Qt::Key_Up)    ||
                        (pKeyEvent->key() == Qt::Key_Down)  )
                {
                    QString fixText = pLineEdit->text();
                    fixup(fixText);
                    setValidate(false);
                    pLineEdit->setText(fixText);
                    setValidate(true);
                    QDate newDate = TextToDate(fixText,m_trennzeichen);
                    pLineEdit->setProperty("date", newDate);
                    emit dateChanged(newDate);
                }
                return false;
            }

            // einen weiter nach links

            if (pKeyEvent->key() == Qt::Key_Left)
            {
               if ( (monatTrenner > 0)  && (pos > monatTrenner) )
               {
                    pLineEdit->setCursorPosition(tagTrenner+1);
                    pLineEdit->setSelection(tagTrenner+1,monatTrenner - tagTrenner - 1);
                    return true;
               }
               else if (pos > tagTrenner)
               {
                    pLineEdit->setCursorPosition(0);
                    pLineEdit->setSelection(0,tagTrenner);
                    return true;
               }
               else
               {
                    if (pLineEdit->selectedText() == pLineEdit->text().left(2))
                    {
                        QString fixText = pLineEdit->text();
                        fixup(fixText);
                        setValidate(false);
                        m_eventFilterOn = true;
                        pLineEdit->setText(fixText);
                        m_eventFilterOn = false;
                        setValidate(true);
                        pLineEdit->setCursorPosition(0);
                        pLineEdit->setSelection(0,0);
                        QDate newDate = TextToDate(fixText,m_trennzeichen);
                        pLineEdit->setProperty("date", newDate);
                        emit dateChanged(newDate);
                        return true;
                    }
                }
            }

            // einen weitere nach rechts

            else if (pKeyEvent->key() == Qt::Key_Right)
            {
                if (pos <= tagTrenner)
                {
                    pLineEdit->setCursorPosition(tagTrenner+1);
                    pLineEdit->setSelection(tagTrenner+1,monatTrenner - tagTrenner - 1);
                    return true;
                }
                else if (pos <= monatTrenner)
                {
                    pLineEdit->setCursorPosition(monatTrenner+1);
                    pLineEdit->setSelection(monatTrenner+1,pLineEdit->text().length() - monatTrenner);
                    return true;
                }
                else
                {
                    if (pLineEdit->selectedText() == pLineEdit->text().mid(monatTrenner+1))
                    {
                        QString fixText = pLineEdit->text();
                        fixup(fixText);
                        setValidate(false);
                        m_eventFilterOn = true;
                        pLineEdit->setText(fixText);
                        m_eventFilterOn = false;
                        setValidate(true);
                        pLineEdit->setCursorPosition(pLineEdit->text().length());
                        pLineEdit->setSelection(pLineEdit->text().length(),0);
                        QDate newDate = TextToDate(fixText,m_trennzeichen);
                        pLineEdit->setProperty("date", newDate);
                        emit dateChanged(newDate);
                        return true;
                     }
                }
            }

            // Pfeil nach oben

            else if (pKeyEvent->key() == Qt::Key_Up)
            {
                bool ok = false;

                if (pLineEdit->selectedText() == pLineEdit->text() )   // der gesamte Text
                {
                    QString teilText = pLineEdit->text().mid(monatTrenner + 1);

                    int jahr = teilText.toInt(&ok);

                    if (jahr < m_jahrhundertGrenze)
                        jahr = 2000 + jahr;
                     else if (jahr < 100)
                        jahr = 1900 + jahr;

                    if (jahr < m_maxDate.year())
                    {
                        jahr++;
                        pLineEdit->setText(pLineEdit->text().left(monatTrenner+1) + QString::number(jahr).left(4));
                        pLineEdit->setCursorPosition(pLineEdit->text().length());
                        pLineEdit->setSelection(0,pLineEdit->text().length());
                        return true;
                    }
                }

                else if (pos <= tagTrenner)
                {
                    QString teilText = pLineEdit->text().left(tagTrenner);

                    int tag = teilText.toInt(&ok);

                    if (tag < 31)
                        tag++;
                    else
                        tag = 1;

                    pLineEdit->setText(QString::number(tag).rightJustified(2,'0',true) + pLineEdit->text().mid(tagTrenner));
                    pLineEdit->setCursorPosition(0);
                    pLineEdit->setSelection(0,tagTrenner);
                    return true;

                }
                else if (pos <= monatTrenner)
                {
                    QString teilText = pLineEdit->text().mid(tagTrenner + 1,monatTrenner - tagTrenner - 1);

                    int monat = teilText.toInt(&ok);

                    if (monat < 12)
                        monat++;
                    else
                        monat = 1;

                    pLineEdit->setText(pLineEdit->text().left(tagTrenner+1) + QString::number(monat).rightJustified(2,'0',true) + pLineEdit->text().mid(monatTrenner));
                    pLineEdit->setCursorPosition(tagTrenner+1);
                    pLineEdit->setSelection(tagTrenner+1,monatTrenner - tagTrenner - 1);
                    return true;
                }
                else // Jahr
                {
                    QString teilText = pLineEdit->text().mid(monatTrenner + 1);

                    int jahr = teilText.toInt(&ok);

                    if (jahr < m_jahrhundertGrenze)
                        jahr = 2000 + jahr;
                     else if (jahr < 100)
                        jahr = 1900 + jahr;

                    if (jahr < m_maxDate.year())
                    {
                        jahr++;
                        pLineEdit->setText(pLineEdit->text().left(monatTrenner+1) + QString::number(jahr).left(4));
                        pLineEdit->setCursorPosition(monatTrenner+1);
                        pLineEdit->setSelection(monatTrenner+1,pLineEdit->text().length() - monatTrenner);
                        return true;
                    }
                }
            }

            // Pfeil nach unten

            else if (pKeyEvent->key() == Qt::Key_Down)
            {
                bool ok = false;

                if (pLineEdit->selectedText() == pLineEdit->text() )   // der gesamte Text
                {
                    QString teilText = pLineEdit->text().mid(monatTrenner + 1);

                    int jahr = teilText.toInt(&ok);

                    if (jahr < m_jahrhundertGrenze)
                        jahr = 2000 + jahr;
                     else if (jahr < 100)
                        jahr = 1900 + jahr;

                    if (jahr > m_minDate.year())
                    {
                        jahr--;
                        pLineEdit->setText(pLineEdit->text().left(monatTrenner+1) + QString::number(jahr).left(4));
                        pLineEdit->setCursorPosition(pLineEdit->text().length());
                        pLineEdit->setSelection(0,pLineEdit->text().length());
                        return true;
                    }
                }

                else if (pos <= tagTrenner)
                {
                    QString teilText = pLineEdit->text().left(tagTrenner);

                    int tag = teilText.toInt(&ok);

                    if (tag > 1)
                        tag--;
                    else
                        tag = 31;

                    pLineEdit->setText(QString::number(tag).rightJustified(2,'0',true) + pLineEdit->text().mid(tagTrenner));
                    pLineEdit->setCursorPosition(0);
                    pLineEdit->setSelection(0,tagTrenner);
                    return true;

                }
                else if (pos <= monatTrenner)
                {
                    QString teilText = pLineEdit->text().mid(tagTrenner + 1,monatTrenner - tagTrenner - 1);

                    int monat = teilText.toInt(&ok);

                    if (monat > 1)
                        monat--;
                    else
                        monat = 12;

                    pLineEdit->setText(pLineEdit->text().left(tagTrenner+1) + QString::number(monat).rightJustified(2,'0',true) + pLineEdit->text().mid(monatTrenner));
                    pLineEdit->setCursorPosition(tagTrenner+1);
                    pLineEdit->setSelection(tagTrenner+1,monatTrenner - tagTrenner - 1);
                    return true;
                }
                else // Jahr
                {
                    QString teilText = pLineEdit->text().mid(monatTrenner + 1);

                    int jahr = teilText.toInt(&ok);

                    if (jahr < m_jahrhundertGrenze)
                        jahr = 2000 + jahr;
                     else if (jahr < 100)
                        jahr = 1900 + jahr;

                    if (jahr > m_minDate.year())
                    {
                        jahr--;
                        pLineEdit->setText(pLineEdit->text().left(monatTrenner+1) + QString::number(jahr).left(4));
                        pLineEdit->setCursorPosition(monatTrenner+1);
                        pLineEdit->setSelection(monatTrenner+1,pLineEdit->text().length() - monatTrenner);
                        return true;
                    }
                }
            }

        }

        // nichts weitere veranlassen

        return false;
    }

    // die normale Eingabe forcieren und immer dann, wenn der Cursor
    // neben zwei Ziffern und auf dem Trennzeichen steht
    // die nächste Spalte (monat, jahr) selectieren

    else if (pEvent->type() == QEvent::KeyRelease)
    {
        QKeyEvent *pKeyEvent = (QKeyEvent*)pEvent;

        // Auf Pfeileingaben nicht nochmal reagieren
        // Auf zwei Key Release hintereinander (ohne Key Press) auch nicht reagieren

        if (    (pKeyEvent->key() == Qt::Key_Up)    ||
                (pKeyEvent->key() == Qt::Key_Down)  ||
                (pKeyEvent->key() == Qt::Key_Left)  ||
                (pKeyEvent->key() == Qt::Key_Right) ||
                (m_lastRelease == true)   )
        {
            return true;
        }

        QLineEdit *pLineEdit = qobject_cast<QLineEdit*>(pObject);
        if (pLineEdit)
        {

            // Eventfiler löschen, sonst rekursion, ausführen
            // und Eventfilter wieder installieren (leider etwas overhead)

            m_eventFilterOn = true;
            pObject->event(pEvent);
            m_eventFilterOn = false;

            // Punkte herausfinden

            int tagTrenner    = pLineEdit->text().indexOf(m_trennzeichen);
            int monatTrenner  = pLineEdit->text().indexOf(m_trennzeichen,tagTrenner + 1);
            int pos           = pLineEdit->cursorPosition();

            if (    (pos == tagTrenner) && (pos == 2)   )
            {
                pLineEdit->setCursorPosition(tagTrenner+1);
                pLineEdit->setSelection(tagTrenner+1, monatTrenner - tagTrenner - 1);
            }
            else if ( (pos == monatTrenner) && (pos == 5)   )
            {
                pLineEdit->setCursorPosition(monatTrenner+1);
                pLineEdit->setSelection(monatTrenner+1, pLineEdit->text().length() - monatTrenner);
            }

            m_lastRelease = true;   // letztes Ereignis war ein release

            return true; // befehl wurde bereits weitergeleitet.
        }
    }

    // Wenn Pfeiltasten vorlieben

    return false;
}


/****************************************************************************
** Methode:     setValidate                                   A.29.09.04	*
**                                                                          *
** Aufgabe:     setzt die statische Membervarialble noValdidate. Wird be-   *
**              nötigt, um Validate im Anschluss an fixup auszuschalten     *
**																			*
** Annahme :    -                                                           *
** Eingabe :    -                                                           *
** Rückgabe:    -                                                           *
** return  :    -                                                           *
*****************************************************************************/


void DateValidator::setValidate(bool on)
{
    this->m_noValidate = !on;
}

/****************************************************************************
** Methode:     SetKeinJahrEnabled                              A.29.09.04	*
**                                                                          *
** Aufgabe:     erlaubt das Vorhandensein von leerem Text, das entspricht   *
**              einem unendlich fernen Datum (intern bei mir der 1.1.4000)  *
**                                                                          *
**              Auf diese Weise ist es möglich einen Zeitraum anzugeben, der*
**              nach hinten offen ist                                       *
**              VONDATUM - BISDATUM (für die meisten Anwendungen sollte     *
**                                   der 1.1.4000 wie unendlich sein        *
**																			*
** Annahme :    -                                                           *
** Eingabe :    -                                                           *
** Rückgabe:    -                                                           *
** return  :    -                                                           *
*****************************************************************************/


void DateValidator::setKeinJahrEnabled(bool enable)
{
    m_keinJahrEnabled = enable;
}


void DateValidator::setCurrentDate(const QDate& currentDate)
{
    m_currentDate = currentDate;
}
