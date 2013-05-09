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

#include "historywindow.h"

// Qt includes

#include <QSplitter>

// Local includes

#include "historypatientlistview.h"


class HistoryWindow::Private
{
public:
    Private()
        : viewSplitter(0),
          patientView(0)
    {
    }

    QSplitter * viewSplitter;
    HistoryPatientListView* patientView;
};

HistoryWindow::HistoryWindow(QWidget *parent)
    : FilterMainWindow(parent),
      d(new Private)
{
    setupToolbar();
    setupView();
}

HistoryWindow::~HistoryWindow()
{
    delete d;
}

void HistoryWindow::entryActivated()
{
}

void HistoryWindow::setupToolbar()
{
}

void HistoryWindow::setupView()
{
    d->viewSplitter = new QSplitter(Qt::Horizontal, this);

    d->patientView = new HistoryPatientListView;
    d->patientView->setAdapter(adapter());

    d->viewSplitter->addWidget(d->patientView);
    //d->viewSplitter->addWidget(d->aggregateView);
}
