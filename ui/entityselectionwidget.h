/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 31.01.2012
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

#ifndef ENTITYSELECTIONWIDGET_H
#define ENTITYSELECTIONWIDGET_H

#include <QGroupBox>

#include "pathology.h"

class EntitySelectionWidget : public QGroupBox
{
    Q_OBJECT
public:

    explicit EntitySelectionWidget(QWidget *parent = 0);
    ~EntitySelectionWidget();

    Pathology::Entity currentEntity() const;

    void applySelectionChange();
    void discardSelectionChange();

signals:

    /// Emitted when the user selects a button.
    /// You must call either apply or discardSelectionChange to react on this signal.
    void selectionChanged(Pathology::Entity entity);

    /// Emitted when the entity changed, either through applySelectionChange,
    /// or from setEntity.
    void entityChanged(Pathology::Entity entity);
    
public slots:

    void setEntity(Pathology::Entity entity);

protected slots:

    void selectionChanged(int);

private:

    class EntitySelectionWidgetPriv;
    EntitySelectionWidgetPriv* const d;
    
};

#endif // ENTITYSELECTIONWIDGET_H
