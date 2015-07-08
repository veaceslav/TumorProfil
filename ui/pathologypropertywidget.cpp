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
#include <QSpinBox>
#include <QVariant>

// Local includes

#include "ihcscore.h"

class PathologyPropertyWidget::PathologyPropertyWidgetPriv
{
public:
    PathologyPropertyWidgetPriv(PathologyPropertyWidget* q)
        : mode(PathologyPropertyInfo::InvalidCategory),
          label(0),
          layout(0),
          radioNP(0),
          radioButtons(0),
          freeInput(0),
          freeInputLabel(0),
          freeInputSuffix(0),
          propertyName("PathologyProperty"),
          q(q)
    {
    }

    QString      property;

    PathologyPropertyInfo::ValueTypeCategory mode;
    QLabel       *label;
    QHBoxLayout  *layout;
    QRadioButton *radioNP;
    QButtonGroup *radioButtons;
    QList<QSpinBox*> spinBoxes;
    QLineEdit    *freeInput;
    QLabel       *freeInputLabel;
    QLabel       *freeInputSuffix;
    QList<QWidget*> otherWidgets;
    const char   *const propertyName;
    PathologyPropertyWidget* const q;

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

    void createPercentageEdit(const QString& text, const QVariant& value)
    {
        QLabel* label = new QLabel(text);
        layout->addWidget(label);
        otherWidgets << label;
        QSpinBox* box = new QSpinBox;
        box->setMinimum(0);
        box->setMaximum(100);
        box->setSingleStep(5);
        box->setProperty(propertyName, value);
        layout->addWidget(box);
        spinBoxes << box;
        connect(box, SIGNAL(valueChanged(int)), q, SLOT(HScoreSpinboxUpdated()));
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
        QObject::connect(freeInput, SIGNAL(textChanged(QString)),
                         q, SLOT(textInserted(QString)));
    }

    void create(const PathologyPropertyInfo& info)
    {
        mode = info.valueType;
        property = info.id;
        radioButtons = new QButtonGroup(q);
        layout = new QHBoxLayout;
        label = new QLabel;
        ValueTypeCategoryInfo typeInfo(info.valueType);
        foreach (const QVariant& value, typeInfo.optionsInUI())
        {
            if (value.isNull() && value.type() == QVariant::Bool)
            {
                createNotPerformed();
            }
            else
            {
                if (typeInfo.category == PathologyPropertyInfo::IHCHScore)
                {
                    createPercentageEdit(typeInfo.toUILabel(value), value);
                }
                else
                {
                    createRadioButton(typeInfo.toUILabel(value), value);
                }
            }

        }
        if (typeInfo.hasDetail())
        {
            QPair<QString,QString> lineEditLabel = typeInfo.defaultDetailLabel();
            createLineEdit(lineEditLabel.first, lineEditLabel.second);
        }

        if (typeInfo.category == PathologyPropertyInfo::IHCTwoDim)
        {
            connect(radioButtons, SIGNAL(buttonClicked(QAbstractButton*)),
                    q, SLOT(twoDimRadioButtonSelection(QAbstractButton*)));
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

};

PathologyPropertyWidget::PathologyPropertyWidget(PathologyPropertyInfo::Property property, QObject *parent)
    : QObject(parent),
      d(new PathologyPropertyWidgetPriv(this))
{
    d->create(PathologyPropertyInfo::info(property));
}

PathologyPropertyWidget::PathologyPropertyWidget(const PathologyPropertyInfo& info, QObject *parent)
    : QObject(parent),
      d(new PathologyPropertyWidgetPriv(this))
{
    d->create(info);
}

PathologyPropertyWidget::~PathologyPropertyWidget()
{
    delete d->label;
    delete d->layout;
    if (d->radioButtons)
        qDeleteAll(d->radioButtons->buttons());
    qDeleteAll(d->spinBoxes);
    qDeleteAll(d->otherWidgets);
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
    if (!d->label)
    {
        return;
    }
    d->label->setText(text);
}

void PathologyPropertyWidget::setDetailLabel(const QString& label)
{
    if (!d->freeInputLabel)
    {
        return;
    }
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

    if (d->mode == PathologyPropertyInfo::IHCHScore)
    {
        bool hasValues = false;
        foreach (QSpinBox* box, d->spinBoxes)
        {
            if (box->value() != 0)
            {
                hasValues = true;
                break;
            }
        }
        qDebug() << "hasValues" << hasValues;
        if (!hasValues)
        {
            return p;
        }

        ValueTypeCategoryInfo typeInfo(d->mode);

        HScore score(d->spinBoxes[3]->value(), d->spinBoxes[2]->value(), d->spinBoxes[1]->value());
        typeInfo.fillHSCore(p, score);
    }
    else
    {
        QRadioButton* checkedButton = static_cast<QRadioButton*>(d->radioButtons->checkedButton());
        if (!checkedButton)
        {
            return p;
        }

        ValueTypeCategoryInfo typeInfo(d->mode);

        p.value = typeInfo.toPropertyValue(checkedButton->property(d->propertyName));
        if (typeInfo.hasDetail())
        {
            p.detail = d->freeInput->text();
        }
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
    ValueTypeCategoryInfo typeInfo(d->mode);
    if (typeInfo.isHScored())
    {
        HScore score(typeInfo.toValue(prop.value));
        QVector<int> ps = score.percentages();
        for (int i=0; i<4; i++)
        {
            d->spinBoxes[i]->setValue(ps[3-i]);
        }
    }
    else
    {
        QRadioButton* toBeChecked = d->findRadioButton(typeInfo.toValue(prop.value));
        if (toBeChecked)
        {
            toBeChecked->setChecked(true);
            if (typeInfo.category == PathologyPropertyInfo::IHCTwoDim)
            {
                twoDimRadioButtonSelection(toBeChecked);
            }
        }
        if (typeInfo.hasDetail() && d->freeInput)
        {
            d->freeInput->setText(prop.detail);
        }
    }
}
/*
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
*/
void PathologyPropertyWidget::textInserted(const QString&)
{
    // E.g., when entering a mutation, set the radio button to Positive
    if (!d->radioButtons->checkedButton() || d->radioButtons->checkedButton() == d->radioNP)
    {
        QAbstractButton* button = d->findRadioButton(true);
        if (button)
        {
            button->setChecked(true);
        }
        else
        {
            button = d->findRadioButton(1);
            if (button)
            {
                button->setChecked(true);
            }
        }
    }
}

void PathologyPropertyWidget::twoDimRadioButtonSelection(QAbstractButton* button)
{
    QVariant value = button->property(d->propertyName);
    if (!d->freeInput)
    {
        return;
    }
    d->freeInput->setEnabled(value.isNull() || value.toInt() != 0);
}

void PathologyPropertyWidget::HScoreSpinboxUpdated()
{
    bool hasValue = false;

    foreach (QSpinBox* box, d->spinBoxes)
    {
        if (box->value()!= 0)
        {
            hasValue = true;
        }
    }
    if (hasValue)
    {
        d->radioButtons->setExclusive(false);
        d->radioNP->setChecked(false);
        d->radioButtons->setExclusive(true);
    }

    d->spinBoxes.first()->setValue(100 - d->spinBoxes[1]->value()
                                       - d->spinBoxes[2]->value()
                                       - d->spinBoxes[3]->value());
}
