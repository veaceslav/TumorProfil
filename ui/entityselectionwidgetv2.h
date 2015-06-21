/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 17.06.2015
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

#ifndef ENTITYSELECTIONWIDGETV2_H
#define ENTITYSELECTIONWIDGETV2_H

#include <QWidget>

class QGroupBox;

#include "pathology.h"

class EntitySelectionWidgetV2 : public QWidget
{
    Q_OBJECT
public:
    explicit EntitySelectionWidgetV2(QWidget *parent = 0);
    ~EntitySelectionWidgetV2();

    Pathology::Entity entity() const;

    // Creates a group box into which this widget is moved
    QGroupBox* createGroupBox();

signals:

    void entityChanged(Pathology::Entity entity);

public slots:

    void setEntity(Pathology::Entity entity);

protected slots:

    void comboBoxIndexChanged(int index);
    void buttonPressed(QAction* action);

private:

    void applyButtonState();
    void applyComboBoxState();

    class EntitySelectionWidgetV2Priv;
    EntitySelectionWidgetV2Priv* const d;
};

#endif // ENTITYSELECTIONWIDGETV2_H
