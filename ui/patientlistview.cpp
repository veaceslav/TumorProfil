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

#include "patientlistview.h"

// Qt includes

#include <QAbstractButton>
#include <QAction>
#include <QContextMenuEvent>
#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QMenu>
#include <QSortFilterProxyModel>

// Local includes

#include "patient.h"
#include "patiententerform.h"
#include "patientmanager.h"
#include "patientmodel.h"

class PatientListviewFilterModel : public QSortFilterProxyModel
{
public:

    PatientListviewFilterModel(QObject* parent)
        : QSortFilterProxyModel(parent),
          onlyTumorProfil(false)
    {
    }

    void setFilterByTumorprofil(bool b)
    {
        onlyTumorProfil = b;
        invalidateFilter();
    }

    virtual bool lessThan ( const QModelIndex & left, const QModelIndex & right ) const
    {
        return QString::localeAwareCompare(left.data(sortRole()).toString(),
                                           right.data(sortRole()).toString()) < 0;
    }

protected:

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
    {
        if (onlyTumorProfil)
        {
            QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
            Patient::Ptr p = PatientModel::retrievePatient(index);
            if ((p && p->hasDisease() && !p->firstDisease().hasProfilePathology())
                    || !p || (p && !p->hasDisease()))
            {
                return false;
            }
        }
        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    }

    bool onlyTumorProfil;
};

class PatientListView::PatientListViewPriv
{
public:
    PatientListViewPriv()
    {
    }

    PatientModel               *model;
    PatientListviewFilterModel *sortFilterModel;
};

PatientListView::PatientListView(QWidget *parent) :
    QTreeView(parent),
    d(new PatientListViewPriv)
{
    d->model = new PatientModel(this);
    d->sortFilterModel = new PatientListviewFilterModel(this);

    d->sortFilterModel->setSourceModel(d->model);
    d->sortFilterModel->setDynamicSortFilter(true);
    d->sortFilterModel->setSortRole(PatientModel::VariantDataRole);
    d->sortFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    setModel(d->sortFilterModel);

    resizeColumnToContents(3);
    resizeColumnToContents(4);
    sortByColumn(0, Qt::AscendingOrder);
    setSortingEnabled(true);

    connect(this, SIGNAL(activated(QModelIndex)),
            this, SLOT(slotActivated(QModelIndex)));
}

QSortFilterProxyModel *PatientListView::filterModel() const
{
    return d->sortFilterModel;
}

Patient::Ptr PatientListView::currentPatient() const
{
    return patientForIndex(currentIndex());
}

void PatientListView::setCurrentPatient(const Patient::Ptr& p)
{
    setCurrentIndex(indexForPatient(p));
}

void PatientListView::slotActivated(const QModelIndex &index)
{
    emit activated(patientForIndex(index));
}

void PatientListView::setFilterByTumorprofil(bool onlyTumorprofil)
{
    d->sortFilterModel->setFilterByTumorprofil(onlyTumorprofil);
}

void PatientListView::contextMenuEvent(QContextMenuEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    if (!index.isValid())
    {
        return;
    }
    Patient::Ptr p = patientForIndex(index);
    if (!p)
    {
        return;
    }

    QMenu menu;

    QAction* editPatientAction = menu.addAction(tr("Patientendaten ändern"), this, SLOT(editPatient()));
    editPatientAction->setData(QVariant::fromValue(p));
    QAction* delPatientAction = menu.addAction(tr("Patienten entfernen"), this, SLOT(deletePatient()));
    delPatientAction->setData(QVariant::fromValue(p));

    menu.exec(event->globalPos());
}

QModelIndex PatientListView::indexForPatient(const Patient::Ptr& patient) const
{
    return d->sortFilterModel->mapFromSource(d->model->indexForPatient(patient));
}

Patient::Ptr PatientListView::patientForIndex(const QModelIndex& index) const
{
    return d->model->patientForIndex(d->sortFilterModel->mapToSource(index));
}

void PatientListView::editPatient()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (!action)
    {
        return;
    }
    Patient::Ptr p = action->data().value<Patient::Ptr>();
    if (!p || !indexForPatient(p).isValid())
    {
        return;
    }

    PatientEnterForm* form = new PatientEnterForm;
    QDialog* dialog = new QDialog;
    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, SIGNAL(accepted()), dialog, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), dialog, SLOT(reject()));
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(form);
    layout->addWidget(buttons);
    dialog->setLayout(layout);

    form->setValues(*p);
    int result = dialog->exec();

    if (result != QDialog::Accepted)
    {
        return;
    }

    p->setPatientData(form->currentPatient());
    PatientManager::instance()->updateData(p);
}

void PatientListView::deletePatient()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (!action)
    {
        return;
    }
    Patient::Ptr p = action->data().value<Patient::Ptr>();
    if (!p || !indexForPatient(p).isValid())
    {
        return;
    }

    QMessageBox box;
    box.setIcon(QMessageBox::Warning);
    box.setText(tr("Das Löschen eines Patienten entfernt alle Daten unwiderruflich"));
    box.setInformativeText(tr("Möchten sie den Patienten \"%1, %2\" wirklich aus der Datenbank entfernen?")
                           .arg(p->surname).arg(p->firstName));
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    box.button(QMessageBox::Yes)->setText(tr("Entfernen"));
    box.button(QMessageBox::Cancel)->setText(tr("Abbrechen"));
    box.setDefaultButton(QMessageBox::Yes);
    if (box.exec() != QMessageBox::Yes)
    {
        return;
    }

    PatientManager::instance()->removePatient(p);

}
