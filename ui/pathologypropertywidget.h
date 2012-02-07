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

#ifndef PATHOLOGYPROPERTYWIDGET_H
#define PATHOLOGYPROPERTYWIDGET_H

// Qt includes

#include <QWidget>

// Local includes

#include "property.h"

class QFormLayout;
class QLabel;

class PathologyPropertyWidget : public QObject
{
    Q_OBJECT
public:

    enum Mode
    {
        IHCClassical,
        IHCBoolean, // "<10%, niedrige Intensität"
        IHCBooleanPercentage,
        Fish,
        Mutation,
        StableUnstable
    };

    explicit PathologyPropertyWidget(const QString& property, Mode mode, QWidget *parent = 0);
    ~PathologyPropertyWidget();

    QString propertyName() const;

    void setLabel(const QString& text);
    void setDetailLabel(const QString& label);
    void setNegativeLabel(const QString& label);
    void setPositiveLabel(const QString& label);

    Property currentProperty();
    /** Returns the initial label and the contant widgets of this widget,
        for use in a separate layout */
    QLabel  *labelWidget();
    QLayout *fieldWidgets();

    /** Add this widget to a QFormLayout */
    void addToLayout(QFormLayout *layout);

    static PathologyPropertyWidget* createIHC(const QString& property, const QString& label);
    static PathologyPropertyWidget* createIHCBoolean(const QString& property, const QString& label);
    static PathologyPropertyWidget* createIHCBooleanPercentage(const QString& property, const QString& label);
    static PathologyPropertyWidget* createFish(const QString& property, const QString& label, const QString& detailLabel);
    static PathologyPropertyWidget* createMutation(const QString& property, const QString& label);
    static PathologyPropertyWidget* createStableUnstable(const QString& property, const QString& label);

public slots:

    void setValue(const Property& prop);

private:

    class PathologyPropertyWidgetPriv;
    PathologyPropertyWidgetPriv* const d;
};

#endif // PATHOLOGYPROPERTYWIDGET_H
