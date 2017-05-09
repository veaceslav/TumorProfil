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
#include "patient.h"
#include "historyelements.h"

class QModelIndex;
class QListWidgetItem;

class HistoryWindow : public FilterMainWindow
{
    Q_OBJECT
public:

    explicit HistoryWindow(QWidget *parent = 0);
    ~HistoryWindow();

    Patient::Ptr currentPatient() const;
    HistoryElement* currentElement() const;
    DiseaseHistory currentHistory() const;

    bool readOnly() const;

public slots:

    void setReadOnly(bool readOnly);

Q_SIGNALS:

    void activated(Patient::Ptr);

protected slots:

    void setCurrentPatient(Patient::Ptr p);
    void setCurrentElement(HistoryElement* e);
    void clearCurrentElement();
    void historyElementActivated(const QModelIndex& index);
    void visualHistoryClicked(HistoryElement* e);

    void addChemotherapy();
    void addFinding();
    void addDiseaseState();
    void addTherapy(QAction*);
    void addTherapy(Therapy::Type type);
    void currentElementChanged();
    void currentElementAddTherapyElement(TherapyElement* te);
    void currentElementTherapyElementChanged(TherapyElement* te);
    void currentElementTherapyElementRemove(TherapyElement* te);
    void proofItemClicked(QListWidgetItem* item);
    void slotHistoryAboutToChange();
    void slotHistoryChanged();
    void slotLastDocumentationDateChanged();

protected:

    virtual void closeEvent(QCloseEvent *);
    virtual void keyPressEvent(QKeyEvent *);
    QDate dateForNewElement() const;
    void applyReadOnlyStatus();

private:

    void setupView();
    void applyData();

    class Private;
    Private* const d;
};

#endif // HISTORYWINDOW_H
