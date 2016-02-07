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

#include "tnmwidget.h"

// Qt includes

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>

class TNMWidget::TNMWidgetPriv
{
public:
    TNMWidgetPriv()
        : editLabel(0),
          tnmEdit(0),
          resultLabel(0)
    {
    }

    QLabel*    editLabel;
    QLineEdit* tnmEdit;
    QLabel*    resultLabel;

    TNM        tnm;
};

TNMWidget::TNMWidget(QWidget *parent) :
    QWidget(parent),
    d(new TNMWidgetPriv)
{
    QVBoxLayout* layout = new QVBoxLayout;

    d->editLabel   = new QLabel(tr("Initiale TNM-Formel (cTNM und/oder pTNM):"));
    d->tnmEdit     = new QLineEdit;
    d->resultLabel = new QLabel;

    layout->addWidget(d->editLabel);
    layout->addWidget(d->tnmEdit);
    layout->addWidget(d->resultLabel);

    connect(d->tnmEdit, SIGNAL(textChanged(QString)),
            this, SLOT(tnmEdited(QString)));
}

TNMWidget::~TNMWidget()
{
    delete d;
}

TNM TNMWidget::currentTNM() const
{
    return d->tnm;
}

void TNMWidget::setTNM(const TNM& tnm)
{
    d->tnm = tnm;
}

void TNMWidget::tnmEdited(const QString& text)
{
    d->tnm.setTNM(text);
}

void TNMWidget::updateResultDisplay()
{
    QString s;
    s += d->tnm.cTNM();
    s += "  ";
    s += d->tnm.pTNM();
    d->resultLabel->setText(s);
}

