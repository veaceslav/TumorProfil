/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 01.08.2015
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

#include "extrainformationtab.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QVBoxLayout>

#include "databaseconstants.h"
#include "pathologypropertyinfo.h"
#include "patientmanager.h"

class ExtraInformationTab::ExtraInformationTabPriv
{
public:
    ExtraInformationTabPriv()
        : editingEnabled(false),
          edited(true)
    {
    }

    Patient::Ptr                              currentPatient;
    bool                                      editingEnabled;
    bool                                      edited;
    QMap<TrialContextInfo::Trial, QCheckBox*> checkboxes;
};

ExtraInformationTab::ExtraInformationTab(QWidget* parent)
    : QWidget(parent),
      d(new ExtraInformationTabPriv)
{
    QVBoxLayout* layout = new QVBoxLayout;

    QGroupBox* box = new QGroupBox(tr("Teilnahme an"));
    QVBoxLayout* boxlayout = new QVBoxLayout;
    for (int t = TrialContextInfo::FirstTrial; t<= TrialContextInfo::LastTrial; ++t)
    {
        TrialContextInfo info((TrialContextInfo::Trial)t);
        QCheckBox* cb = new QCheckBox(info.label);
        d->checkboxes[info.trial] = cb;
        boxlayout->addWidget(cb);
    }
    boxlayout->addStretch();
    box->setLayout(boxlayout);

    layout->addWidget(box);
    setLayout(layout);
}

ExtraInformationTab::~ExtraInformationTab()
{
    delete d;
}

void ExtraInformationTab::save()
{
    if (!d->editingEnabled)
    {
        return;
    }

    QMap<TrialContextInfo::Trial, QCheckBox*>::const_iterator it;
    for (it = d->checkboxes.begin(); it != d->checkboxes.end(); ++it)
    {
        TrialContextInfo info(it.key());
        //qDebug() << "Set property" << info.id << (it.value()->isChecked());
        if (it.value()->isChecked())
        {
            d->currentPatient->patientProperties.setProperty(PatientPropertyName::trialParticipation(),
                                                             info.id);
        }
        else
        {
            d->currentPatient->patientProperties.removeProperty(PatientPropertyName::trialParticipation(),
                                                                info.id);
        }
    }
    PatientManager::instance()->updateData(d->currentPatient, PatientManager::ChangedPatientProperties);
}

void ExtraInformationTab::setDisease(const Patient::Ptr& p, int)
{
    if (d->currentPatient == p)
    {
        return;
    }

    setEnabled(p);
    d->edited = false;
    if (!p)
    {
        return;
    }

    QMap<TrialContextInfo::Trial, QCheckBox*>::const_iterator it;
    for (it = d->checkboxes.begin(); it != d->checkboxes.end(); ++it)
    {
        TrialContextInfo info(it.key());
        it.value()->setChecked(
                    p->patientProperties.hasProperty(PatientPropertyName::trialParticipation(),
                                                     info.id)
                    );
    }
}

QString ExtraInformationTab::tabLabel() const
{
    return tr("Studien");
}

void ExtraInformationTab::setEditingEnabled(bool enabled)
{
    foreach (QCheckBox* box, d->checkboxes)
    {
        box->setEnabled(enabled);
    }
}

void ExtraInformationTab::checkBoxChanged()
{
    d->edited = true;
}
