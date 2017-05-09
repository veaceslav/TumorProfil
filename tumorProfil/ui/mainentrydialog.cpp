/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 2017-05-09
 * Description : Dialog to prompt users about main action
 *
 * Copyright (C) 2010-2012, 2017 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2013-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "mainentrydialog.h"

// Qt includes

#include <QToolBar>
#include <QToolButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QPushButton>
#include <QButtonGroup>
#include <QStyle>
#include <QApplication>

// Local includes

#include "authentication/accessmanagement.h"
#include "mainwindow.h"

class MainEntryDialog::Private
{
public:

    Private()
        : buttonGroup(0),
          iconSize(22),
          secondSeparator(0),
          action(MainEntryDialog::Cancel)
    {
    }

    void addButton(int key, const QString& iconName, const QString& text)
    {
       QToolButton* const button = new QToolButton;
       button->setText(text);
       button->setIcon(QIcon::fromTheme(iconName));
       button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
       button->setAutoRaise(true);
       button->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

       buttonGroup->addButton(button, key);

       toolBar->insertWidget(secondSeparator, button);
    }

    void createToolBar()
    {
        toolBar         = new QToolBar;
        toolBar->setOrientation(Qt::Vertical);
        toolBar->setIconSize(QSize(iconSize, iconSize));
        toolBar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
        toolBar->addSeparator();
        secondSeparator = toolBar->addSeparator();
    }

public:

    QToolBar*     toolBar;
    QButtonGroup* buttonGroup;
    int           iconSize;
    QAction*      secondSeparator;

    MainEntryDialog::Action action;
};

MainEntryDialog::MainEntryDialog(QWidget* const parent)
    : QDialog(parent),
      d(new Private)
{
    d->buttonGroup = new QButtonGroup(this);
    d->iconSize = 64;//style()->pixelMetric(QStyle::PM_MessageBoxIconSize, 0, this);
    setWindowTitle(tr("Tumorprofil Datenbank"));
    d->createToolBar();

    d->addButton(EditWindow, "application-form-edit", "Befundeingabe");
    d->addButton(HistoryWindow, "calendar-edit", "Krankheitsgeschichte bearbeiten");
    d->addButton(HistoryWindowReadOnly, "calendar", "Krankheitsgeschichte ansehen");
    d->addButton(ReportWindow, "report", "Datenabfrage");
    d->addButton(Cancel, "cancel", "Beenden");

    connect(d->buttonGroup, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
            this, &MainEntryDialog::slotButtonClicked);

    QVBoxLayout* const layout = new QVBoxLayout;
    layout->addWidget(d->toolBar);
    setLayout(layout);
}

MainEntryDialog::~MainEntryDialog()
{
    delete d;
}

void MainEntryDialog::setEnabled(Action action, bool enabled)
{
    QAbstractButton* button = d->buttonGroup->button(action);
    if (button)
    {
        button->setEnabled(enabled);
    }
}

void MainEntryDialog::setEnabledFromAccessRights()
{
    AccessManagement::AccessType pathoAccess = AccessManagement::accessToPathologyData();
    AccessManagement::AccessType histoAccess = AccessManagement::accessToDiseaseHistory();
    d->buttonGroup->button(EditWindow)->setEnabled(pathoAccess == AccessManagement::ReadWrite);
    d->buttonGroup->button(HistoryWindow)->setEnabled(histoAccess == AccessManagement::ReadWrite);
    d->buttonGroup->button(HistoryWindowReadOnly)->setEnabled(histoAccess & AccessManagement::Read);
    d->buttonGroup->button(ReportWindow)->setEnabled(pathoAccess & AccessManagement::Read);
}

void MainEntryDialog::slotButtonClicked(int id)
{
    d->action = Action(id);
    executeAction(d->action);
}

MainEntryDialog::Action MainEntryDialog::action() const
{
    return d->action;
}

void MainEntryDialog::executeAction(MainEntryDialog::Action action)
{
    switch (action)
    {
    case EditWindow:
    {
        MainWindow::showMainWindow();
        break;
    }
    case ReportWindow:
        MainWindow::showReportWindow();
        break;
    case HistoryWindow:
        MainWindow::showHistoryWindow(true);
        break;
    case HistoryWindowReadOnly:
        MainWindow::showHistoryWindow(false);
        break;
    case Cancel:
        qApp->exit(0);
    }
}


