/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 07.05.2012
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

#include "pathologymetadatawidget.h"

// Local includes

// Qt includes

#include <QButtonGroup>
#include <QDateEdit>
#include <QFormLayout>
#include <QGridLayout>
#include <QRadioButton>
#include <QVBoxLayout>

class PathologyMetadataWidget::PathologyMetadataWidgetPriv
{
public:

    PathologyMetadataWidgetPriv()
        : dateEdit(0),
          sampleFromPrimaryButton(0),
          sampleFromMetastasisButton(0),
          sampleFromLNButton(0),
          sampleOriginGroup(0)

    {
    }

    QDateEdit             *dateEdit;
    QRadioButton          *sampleFromPrimaryButton;
    QRadioButton          *sampleFromMetastasisButton;
    QRadioButton          *sampleFromLNButton;
    QButtonGroup          *sampleOriginGroup;
};

PathologyMetadataWidget::PathologyMetadataWidget(QWidget *parent) :
    QWidget(parent),
    d(new PathologyMetadataWidgetPriv)
{
    QGridLayout* layout = new QGridLayout;

    // Sample origin buttons
    QFormLayout* buttonLayout = new QFormLayout;
    d->sampleFromPrimaryButton = new QRadioButton(tr("aus dem Primärtumor"));
    d->sampleFromLNButton = new QRadioButton(tr("aus einem lokalen Lymphknoten"));
    d->sampleFromMetastasisButton = new QRadioButton(tr("aus einer Metastase"));
    buttonLayout->addRow(tr("Histologie"), d->sampleFromPrimaryButton);
    buttonLayout->addRow(QString(), d->sampleFromLNButton);
    buttonLayout->addRow(QString(), d->sampleFromMetastasisButton);
    d->sampleOriginGroup = new QButtonGroup(this);
    d->sampleOriginGroup->addButton(d->sampleFromPrimaryButton, Pathology::Primary);
    d->sampleOriginGroup->addButton(d->sampleFromLNButton, Pathology::LocalLymphNode);
    d->sampleOriginGroup->addButton(d->sampleFromMetastasisButton, Pathology::Metastasis);

    QFormLayout* dateLayout = new QFormLayout;
    d->dateEdit = new QDateEdit;
    dateLayout->addRow(tr("Befunddatum:"), d->dateEdit);

    layout->addLayout(buttonLayout, 0, 0);
    layout->addLayout(dateLayout, 0, 1);

    setLayout(layout);

    setFocusProxy(d->dateEdit);
}

PathologyMetadataWidget::~PathologyMetadataWidget()
{
    delete d;
}

void PathologyMetadataWidget::reset()
{
    if (d->sampleOriginGroup->checkedButton())
    {
        d->sampleOriginGroup->setExclusive(false);
        d->sampleOriginGroup->checkedButton()->setChecked(false);
        d->sampleOriginGroup->setExclusive(true);
    }
    d->dateEdit->setDate(QDate::currentDate());
}

void PathologyMetadataWidget::readValues(const Pathology& path)
{
    if (d->sampleOriginGroup->button(path.sampleOrigin))
    {
        d->sampleOriginGroup->button(path.sampleOrigin)->setChecked(true);
    }
    if (path.date.isValid())
    {
        d->dateEdit->setDate(path.date);
    }
}

void PathologyMetadataWidget::saveValues(Pathology& path)
{
    /*Pathology::SampleOrigin sampleOrigin = Pathology::UnknownOrigin;
    if (disease.hasProfilePathology())
    {
        sampleOrigin = disease.firstProfilePathology().sampleOrigin;
    }
    else if (disease.hasPathology())
    {
        sampleOrigin = disease.firstPathology().sampleOrigin;
    }*/

    path.sampleOrigin = d->sampleOriginGroup->checkedButton()
            ? (Pathology::SampleOrigin)d->sampleOriginGroup->checkedId() : Pathology::UnknownOrigin;
    path.date = d->dateEdit->date();
}

