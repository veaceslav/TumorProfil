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

#include "pathologypropertywidget.h"

// Qt includes

#include <QButtonGroup>
#include <QDebug>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QVariant>

class PathologyPropertyWidget::PathologyPropertyWidgetPriv
{
public:
    PathologyPropertyWidgetPriv()
        : label(0),
          layout(0),
          radioNP(0),
          radioButtons(0),
          freeInput(0),
          freeInputLabel(0),
          freeInputSuffix(0),
          propertyName("PathologyProperty")
    {
    }

    QString      property;

    PathologyPropertyWidget::Mode mode;
    QLabel       *label;
    QHBoxLayout  *layout;
    QRadioButton *radioNP;
    QButtonGroup *radioButtons;
    QLineEdit    *freeInput;
    QLabel       *freeInputLabel;
    QLabel       *freeInputSuffix;
    const char   *const propertyName;

    void createNotPerformed()
    {
        radioNP = new QRadioButton(QObject::tr("n.d."));
        radioNP->setChecked(true);
        radioNP->setProperty(propertyName, QVariant(QVariant::Bool));
        layout->addWidget(radioNP);
        radioButtons->addButton(radioNP);
    }

    void createRadioButton(const QString& text, const QVariant& value)
    {
        QRadioButton* radio = new QRadioButton(text);
        radio->setProperty(propertyName, value);
        layout->addWidget(radio);
        radioButtons->addButton(radio);
    }

    /// Pass a null string if you dont want the label
    /// Pass an empty string if you want the label, but dont know the text yet
    void createLineEdit(const QString& label, const QString& suffix)
    {
        if (!label.isNull())
        {
            freeInputLabel = new QLabel(label);
            layout->addWidget(freeInputLabel);
        }
        freeInput = new QLineEdit;
        layout->addWidget(freeInput);
        if (!label.isEmpty())
        {
            freeInputSuffix = new QLabel(suffix);
            layout->addWidget(freeInputSuffix);
        }
    }

    void create()
    {
        layout = new QHBoxLayout;
        label = new QLabel;
        switch (mode)
        {
        case IHCClassical:
            createNotPerformed();
            createRadioButton(tr("0"), 0);
            createRadioButton(tr("1+"), 1);
            createRadioButton(tr("2+"), 2);
            createRadioButton(tr("3+"), 3);
            break;
        case IHCBoolean:
            createNotPerformed();
            createRadioButton(tr("negativ (<10%, niedrige Intensität)"), false);
            createRadioButton(tr("positiv"), true);
            break;
        case IHCBooleanPercentage:
            createNotPerformed();
            createRadioButton(tr("negativ"), false);
            createLineEdit(QString(), tr("% Zellen"));
            createRadioButton(tr("positiv"), true);
            break;
        case Fish:
            createNotPerformed();
            createRadioButton(tr("negativ"), false);
            createRadioButton(tr("positiv"), true);
            createLineEdit("", QString());
            break;
        case Mutation:
            createNotPerformed();
            createRadioButton(tr("negativ"), false);
            createRadioButton(tr("positiv"), true);
            createLineEdit(tr("Mutation:"), QString());
            break;
        case StableUnstable:
            createNotPerformed();
            createRadioButton(tr("stabil"), false);
            createRadioButton(tr("instabil"), true);
            break;
        }
        layout->addStretch();
    }

    QRadioButton* findRadioButton(const QVariant& value)
    {
        foreach (QAbstractButton* radio, radioButtons->buttons())
        {
            QVariant property = radio->property(propertyName);
            if (property.type() == value.type() &&
                    property.isNull() == value.isNull() &&
                    property == value)
            {
                return static_cast<QRadioButton*>(radio);
            }
        }
        return 0;
    }

    static QString boolToString(const QVariant& var)
    {
        return boolToString(var.toBool());
    }

    static QString boolToString(bool b)
    {
        if (b)
        {
            return "1";
        }
        else
        {
            return "0";
        }
    }

    static bool stringToBool(const QString& s)
    {
        if (s == "1" || s == "true" || s == "pos" || s.startsWith("positiv") || s == "+")
        {
            return true;
        }
        if (s == "0" || s == "false" || s == "neg" || s.startsWith("negativ") || s == "-")
        {
            return false;
        }
        bool ok;
        int i = s.toInt(&ok);
        if (ok)
        {
            return i;
        }
        return false;
    }
};

PathologyPropertyWidget::PathologyPropertyWidget(const QString& property, Mode mode, QWidget *parent) :
    QObject(parent),
    d(new PathologyPropertyWidgetPriv)
{
    d->mode = mode;
    d->property = property;
    d->radioButtons = new QButtonGroup(this);
    d->create();
}

PathologyPropertyWidget::~PathologyPropertyWidget()
{
    delete d->label;
    delete d->layout;
    if (d->radioButtons)
        qDeleteAll(d->radioButtons->buttons());
    delete d->radioButtons;
    delete d->freeInput;
    delete d->freeInputLabel;
    delete d->freeInputSuffix;
    delete d;
}

QString PathologyPropertyWidget::propertyName() const
{
    return d->property;
}

QLabel  *PathologyPropertyWidget::labelWidget()
{
    return d->label;
}

QLayout *PathologyPropertyWidget::fieldWidgets()
{
    return d->layout;
}

void PathologyPropertyWidget::addToLayout(QFormLayout *layout)
{
    layout->addRow(d->label, d->layout);
}

void PathologyPropertyWidget::setLabel(const QString& text)
{
    d->label->setText(text);
}

void PathologyPropertyWidget::setDetailLabel(const QString& label)
{
    d->freeInputLabel->setText(label);
}

void PathologyPropertyWidget::setNegativeLabel(const QString& label)
{
    QRadioButton* radio = d->findRadioButton(QVariant(bool(false)));
    if (radio)
    {
        radio->setText(label);
    }
}

void PathologyPropertyWidget::setPositiveLabel(const QString& label)
{
    QRadioButton* radio = d->findRadioButton(QVariant(bool(true)));
    if (radio)
    {
        radio->setText(label);
    }
}

Property PathologyPropertyWidget::currentProperty()
{
    Property p;
    p.property = d->property;
    if (d->radioNP && d->radioNP->isChecked())
    {
        return p;
    }

    QRadioButton* checkedButton = static_cast<QRadioButton*>(d->radioButtons->checkedButton());

    switch (d->mode)
    {
    case IHCClassical:
        p.value = checkedButton->property(d->propertyName).toString();
        break;
    case IHCBoolean:
        p.value = d->boolToString(checkedButton->property(d->propertyName));
        break;
    case IHCBooleanPercentage:
        p.value = d->boolToString(checkedButton->property(d->propertyName));
        p.detail = d->freeInput->text();
        break;
    case Fish:
        p.value = d->boolToString(checkedButton->property(d->propertyName));
        p.detail = d->freeInput->text();
        break;
    case Mutation:
        p.value = d->boolToString(checkedButton->property(d->propertyName));
        p.detail = d->freeInput->text();
        break;
    case StableUnstable:
        p.value = d->boolToString(checkedButton->property(d->propertyName));
        break;
    }
    return p;
}

void PathologyPropertyWidget::setValue(const Property& prop)
{
    if (prop.property != d->property)
    {
        qWarning() << "Property mismatch, widget for" << d->property << "fed with" << prop.property;
        return;
    }

    QVariant value;
    QRadioButton* toBeChecked = 0;
    switch (d->mode)
    {
    case IHCClassical:
        toBeChecked = d->findRadioButton(prop.value.toInt());
        break;
    case IHCBoolean:
        toBeChecked = d->findRadioButton(d->stringToBool(prop.value));
        break;
    case IHCBooleanPercentage:
        toBeChecked = d->findRadioButton(d->stringToBool(prop.value));
        break;
    case Fish:
        toBeChecked = d->findRadioButton(d->stringToBool(prop.value));
        break;
    case Mutation:
        toBeChecked = d->findRadioButton(d->stringToBool(prop.value));
        break;
    case StableUnstable:
        toBeChecked = d->findRadioButton(d->stringToBool(prop.value));
        break;
    }

    if (d->freeInput)
    {
        d->freeInput->setText(prop.detail);
    }
    if (toBeChecked)
    {
        toBeChecked->setChecked(true);
    }
}

PathologyPropertyWidget* PathologyPropertyWidget::createIHC(const QString& property, const QString& label)
{
    PathologyPropertyWidget* w = new PathologyPropertyWidget(property, PathologyPropertyWidget::IHCClassical);
    w->setLabel(label);
    return w;
}

PathologyPropertyWidget* PathologyPropertyWidget::createIHCBoolean(const QString& property,
                                                                   const QString& label)
{
    PathologyPropertyWidget* w = new PathologyPropertyWidget(property,
                                                             PathologyPropertyWidget::IHCBoolean);
    w->setLabel(label);
    return w;
}

PathologyPropertyWidget* PathologyPropertyWidget::createIHCBooleanPercentage(const QString& property,
                                                                             const QString& label)
{
    PathologyPropertyWidget* w = new PathologyPropertyWidget(property,
                                                             PathologyPropertyWidget::IHCBooleanPercentage);
    w->setLabel(label);
    return w;
}

PathologyPropertyWidget* PathologyPropertyWidget::createFish(const QString& property, const QString& label,
                                                             const QString& detailLabel)
{
    PathologyPropertyWidget* w = new PathologyPropertyWidget(property,
                                                             PathologyPropertyWidget::Fish);
    w->setLabel(label);
    w->setDetailLabel(detailLabel);
    return w;
}

PathologyPropertyWidget* PathologyPropertyWidget::createMutation(const QString& property, const QString& label)
{
    PathologyPropertyWidget* w = new PathologyPropertyWidget(property,
                                                             PathologyPropertyWidget::Mutation);
    w->setLabel(label);
    return w;
}

PathologyPropertyWidget* PathologyPropertyWidget::createStableUnstable(const QString& property,
                                                                       const QString& label)
{
    PathologyPropertyWidget* w = new PathologyPropertyWidget(property,
                                                             PathologyPropertyWidget::StableUnstable);
    w->setLabel(label);
    return w;
}



