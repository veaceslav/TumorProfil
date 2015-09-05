/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 03.08.2015
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

#include "pathologyreporttab.h"

#include <QGridLayout>
#include <QLabel>
#include <QListWidget>
#include <QTextEdit>

class PathologyReportTab::PathologyReportTabPriv
{
public:
    PathologyReportTabPriv()
        : dateList(0),
          textEdit(0)
    {
    }

    QListWidget*        dateList;
    QTextEdit*          textEdit;

    QStringList         reports;
};

PathologyReportTab::PathologyReportTab(QWidget *parent)
    : QWidget(parent),
      d(new PathologyReportTabPriv)
{
    QGridLayout* layout = new QGridLayout;

    QLabel* reportLabel = new QLabel(tr("Befunde:"));
    layout->addWidget(reportLabel, 0, 0);
    QLabel* textLabel   = new QLabel(tr("Inhalt:"));
    layout->addWidget(textLabel, 0, 1);

    d->dateList = new QListWidget;
    layout->addWidget(d->dateList, 1, 0);

    d->textEdit = new QTextEdit;
    d->textEdit->setReadOnly(true);
    layout->addWidget(d->textEdit, 1, 1);

    layout->setRowStretch(1, 1);
    layout->setColumnStretch(0, 2);
    layout->setColumnStretch(1, 8);

    setLayout(layout);

    connect(d->dateList, &QListWidget::currentRowChanged, this, &PathologyReportTab::reportSelected);
}

PathologyReportTab::~PathologyReportTab()
{
    delete d;
}

void PathologyReportTab::setPathologies(const QList<Pathology> &pathologies)
{
    d->dateList->clear();
    d->textEdit->clear();
    d->reports.clear();

    foreach (const Pathology& pathology, pathologies)
    {
        foreach (const QString& text, pathology.reports)
        {
            d->dateList->addItem(pathology.date.toString(tr("dd.MM.yyyy")));
            d->reports << text;
        }
    }
    if (d->dateList->count())
    {
        d->dateList->setCurrentRow(0);
    }
}

void PathologyReportTab::reportSelected(int index)
{
    if (index < 0 || index >= d->reports.size())
    {
        d->textEdit->clear();
        return;
    }

    d->textEdit->setPlainText(d->reports[index]);
}
