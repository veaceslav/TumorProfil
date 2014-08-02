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

#include "filtermainwindow.h"

// Qt includes

#include <QAction>
#include <QComboBox>
#include <QDateEdit>
#include <QDebug>
#include <QIcon>
#include <QMenu>
#include <QToolBar>
#include <QToolButton>
#include <QWidgetAction>

// Local includes

#include "pathologypropertyinfo.h"
#include "patientpropertymodel.h"
#include "patientpropertyfiltermodel.h"
#include "patientpropertymodelviewadapter.h"

class FilterMainWindow::FilterMainWindowPriv
{
public:
    FilterMainWindowPriv()
        : adapter(0),
         toolBar(0),
         reportComboBox(0),
         fromEdit(0),
         toEdit(0)
    {
    }

    PatientPropertyModelViewAdapter *adapter;

    QToolBar   *toolBar;
    QComboBox  *reportComboBox;

    QDateEdit  *fromEdit;
    QDateEdit  *toEdit;

    QList<QAction*> contextFilterActions;
};

FilterMainWindow::FilterMainWindow(QWidget *parent) :
    QMainWindow(parent),
    d(new FilterMainWindowPriv)
{
    d->adapter = new PatientPropertyModelViewAdapter(this);

    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->adapter->setReportType(PatientPropertyModelViewAdapter::OverviewReport);
}

FilterMainWindow::~FilterMainWindow()
{
    delete d;
}

PatientPropertyModelViewAdapter* FilterMainWindow::adapter() const
{
    return d->adapter;
}

QToolBar* FilterMainWindow::toolBar() const
{
    return d->toolBar;
}

class ProfileMenu : public QMenu
{
public:
    QAction* addAction(const QString& text, int userData, QObject* receiver,
                       const char* slot = SLOT(entryActivated()), bool isCheckable = false)
    {
        QAction* action = QMenu::addAction(text);
        action->setData(userData);
        if (isCheckable)
        {
            action->setCheckable(true);
            connect(action, SIGNAL(toggled(bool)), receiver, slot);
        }
        else
        {
            connect(action, SIGNAL(triggered()), receiver, slot);
        }
        return action;
    }
};

void FilterMainWindow::setupToolbar()
{
    d->toolBar = addToolBar(tr("Auswahl"));

    QAction* overviewAction = d->toolBar->addAction(QIcon::fromTheme("table"), tr("Übersicht"),
                                                    this, SLOT(entryActivated()));
    overviewAction->setData((int)PatientPropertyModelViewAdapter::OverviewReport);

    QAction* profilesAction = d->toolBar->addAction(QIcon::fromTheme("photo"), tr("Nach Entität"));
    QToolButton* profilesButton = static_cast<QToolButton*>(d->toolBar->widgetForAction(profilesAction));
    profilesButton->setPopupMode(QToolButton::InstantPopup);
    ProfileMenu* profilesMenu = new ProfileMenu;
    profilesMenu->addAction(tr("Adenokarzinom Lunge"), PatientPropertyModelViewAdapter::PulmonaryAdenoIHCMut, this);
    profilesMenu->addAction(tr("Plattenepithelkarzinom Lunge"), PatientPropertyModelViewAdapter::PulmonarySquamousIHCMut, this);
    profilesMenu->addAction(tr("Kolorektales Karzinom"), PatientPropertyModelViewAdapter::CRCIHCMut, this);
    profilesMenu->addAction(tr("Mammakarzinom"), PatientPropertyModelViewAdapter::BreastCaIHCMut, this);
    profilesMenu->addAction(tr("Alle vier Tumorprofil-Entitäten"), PatientPropertyModelViewAdapter::TumorprofilIHCMut, this);
    profilesButton->setMenu(profilesMenu);

    QAction* byMutationAction = d->toolBar->addAction(QIcon::fromTheme("palette"), tr("Nach Befund"));
    QToolButton* byMutationButton = static_cast<QToolButton*>(d->toolBar->widgetForAction(byMutationAction));
    byMutationButton->setPopupMode(QToolButton::InstantPopup);
    ProfileMenu* byMutationMenu = new ProfileMenu;
    byMutationMenu->addAction(tr("EGFR-Mutation"), PatientPropertyModelViewAdapter::EGFRMutation, this);
    byMutationMenu->addAction(tr("PIK3CA-Mutation"), PatientPropertyModelViewAdapter::PIK3Mutation, this);
    byMutationMenu->addAction(tr("PTEN-Verlust"), PatientPropertyModelViewAdapter::PTENLoss, this);
    byMutationMenu->addAction(tr("BRAF-Mutation"), PatientPropertyModelViewAdapter::BRAFMutation, this);
    byMutationMenu->addAction(tr("ALK-Amplifikation"), PatientPropertyModelViewAdapter::ALKAmplification, this);
    byMutationMenu->addAction(tr("NSCLC KRAS-Mutation"), PatientPropertyModelViewAdapter::NSCLCKRASMutation, this);
    byMutationMenu->addAction(tr("NSCLC Her2-Amplifikation"), PatientPropertyModelViewAdapter::NSCLCHer2Amplification, this);
    byMutationMenu->addAction(tr("NSCLC cMET-Überexpression"), PatientPropertyModelViewAdapter::NSCLCcMetOverexpression, this);
    byMutationMenu->addAction(tr("CRC cMET-Überexpression"), PatientPropertyModelViewAdapter::CRCcMetOverexpression, this);
    byMutationButton->setMenu(byMutationMenu);

    d->toolBar->addSeparator();

    QAction* byContextAction = d->toolBar->addAction(QIcon::fromTheme("folder"), tr("Filter nach Kontext"));
    QToolButton* byContextButton = static_cast<QToolButton*>(d->toolBar->widgetForAction(byContextAction));
    byContextButton->setPopupMode(QToolButton::InstantPopup);
    ProfileMenu* byContextMenu = new ProfileMenu;
    d->contextFilterActions <<
        byContextMenu->addAction(tr("WTZ-Tumorprofil"),  PathologyContextInfo::Tumorprofil,
                                 this, SLOT(filterByContext()), true);
    d->contextFilterActions <<
        byContextMenu->addAction(tr("BestRx"), PathologyContextInfo::BestRx,
                                 this, SLOT(filterByContext()), true);
    d->contextFilterActions <<
        byContextMenu->addAction(tr("BEZ235-Screening"), PathologyContextInfo::ScreeningBEZ235,
                                 this, SLOT(filterByContext()), true);
    d->contextFilterActions <<
        byContextMenu->addAction(tr("BGJ389-Screening"), PathologyContextInfo::ScreeningBGJ398,
                                 this, SLOT(filterByContext()), true);
    d->contextFilterActions <<
        byContextMenu->addAction(tr("BKM120-Screening"), PathologyContextInfo::ScreeningBKM120,
                                 this, SLOT(filterByContext()), true);
    d->contextFilterActions <<
        byContextMenu->addAction(tr("PemSplitCisp-Studie"), TrialContextInfo::AIO_TRK_0212,
                                 this, SLOT(filterByTrial()), true);
    d->contextFilterActions <<
        byContextMenu->addAction(tr("Nur lokale Patienten"), PatientPropertyFilterSettings::LocalCenterOrigin,
                                 this, SLOT(filterByCriteria()), true);
    d->contextFilterActions <<
        byContextMenu->addAction(tr("(kein Filter)"), PathologyContextInfo::InvalidContext,
                                 this, SLOT(filterByContext()), false);
    byContextButton->setMenu(byContextMenu);

    d->toolBar->addSeparator();

    QAction* byDateAction = d->toolBar->addAction(QIcon::fromTheme("calendar"), tr("Nach Datum"));
    QToolButton* byDateButton = static_cast<QToolButton*>(d->toolBar->widgetForAction(byDateAction));
    byDateButton->setPopupMode(QToolButton::InstantPopup);
    QMenu* byDateMenu = new QMenu;

    byDateMenu->addAction(tr("Datumsfilter zurücksetzen"), this, SLOT(clearDateFilter()));

    d->fromEdit = new QDateEdit;
    connect(d->fromEdit, SIGNAL(dateChanged(QDate)), this, SLOT(filterByDate()));
    QWidgetAction* fromAction = new QWidgetAction(this);
    fromAction->setDefaultWidget(d->fromEdit);
    byDateMenu->addAction(fromAction);

    d->toEdit = new QDateEdit;
    d->toEdit->setDate(QDate(2016,1,1));
    connect(d->toEdit, SIGNAL(dateChanged(QDate)), this, SLOT(filterByDate()));
    QWidgetAction* toAction = new QWidgetAction(this);
    toAction->setDefaultWidget(d->toEdit);
    byDateMenu->addAction(toAction);

    byDateButton->setMenu(byDateMenu);
}

void FilterMainWindow::filterByContext()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action)
    {
        return;
    }
    PatientPropertyFilterSettings settings = d->adapter->filterModel()->filterSettings();
    PathologyContextInfo::Context context = (PathologyContextInfo::Context)action->data().toInt();
    if (context == PathologyContextInfo::InvalidContext)
    {
        settings.pathologyContexts.clear();
        settings.trialParticipation.clear();
        settings.criteria.clear();
        foreach (QAction* a, d->contextFilterActions)
        {
            a->setChecked(false);
        }
    }
    else
    {
        PathologyContextInfo info(context);
        if (action->isChecked())
        {
            settings.pathologyContexts[info.id] = true;
        }
        else
        {
            settings.pathologyContexts.remove(info.id);
        }
    }
    d->adapter->filterModel()->setFilterSettings(settings);
}

void FilterMainWindow::filterByTrial()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action)
    {
        return;
    }
    PatientPropertyFilterSettings settings = d->adapter->filterModel()->filterSettings();
    TrialContextInfo::Trial context = (TrialContextInfo::Trial)action->data().toInt();

    TrialContextInfo info(context);
    if (action->isChecked())
    {
        settings.trialParticipation[info.id] = true;
    }
    else
    {
        settings.trialParticipation.remove(info.id);
    }
    d->adapter->filterModel()->setFilterSettings(settings);
}

void FilterMainWindow::filterByDate()
{
    PatientPropertyFilterSettings settings = d->adapter->filterModel()->filterSettings();
    settings.resultDateBegin = d->fromEdit->date();
    settings.resultDateEnd = d->toEdit->date();
    d->adapter->filterModel()->setFilterSettings(settings);
}

void FilterMainWindow::clearDateFilter()
{
    PatientPropertyFilterSettings settings = d->adapter->filterModel()->filterSettings();
    settings.resultDateBegin = QDate();
    settings.resultDateEnd = QDate();
    d->adapter->filterModel()->setFilterSettings(settings);
}

void FilterMainWindow::filterByCriteria()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action)
    {
        return;
    }
    PatientPropertyFilterSettings settings = d->adapter->filterModel()->filterSettings();
    PatientPropertyFilterSettings::Criteria c = (PatientPropertyFilterSettings::Criteria)action->data().toInt();
    if (action->isChecked())
    {
        settings.criteria[c] = true;
    }
    else
    {
        settings.criteria.remove(c);
    }
    d->adapter->filterModel()->setFilterSettings(settings);
}

void FilterMainWindow::entryActivated()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action)
    {
        return;
    }
    d->adapter->setReportType(action->data().toInt());
}

