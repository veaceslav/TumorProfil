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

#ifndef PATHOLOGYWIDGETGENERATOR_H
#define PATHOLOGYWIDGETGENERATOR_H

// Qt includes

#include <QHash>
#include <QObject>

// Local includes

#include "pathology.h"
#include "pathologypropertyinfo.h"

class QFormLayout;

class PathologyPropertyWidget;

class PathologyWidgetGenerator : public QObject
{
public:

    PathologyWidgetGenerator();
    ~PathologyWidgetGenerator();

    /**
      Rebuilds the widgets only if the entity differs.
      Returns true if widgets were recreated.
      */
    bool switchEntity(Pathology::Entity entity);
    bool switchEntity(Pathology::Entity entity, QFormLayout* layout);

    /** Returns a mixed list of label widgets and PathologyPropertyWigdets
        according to the entity of this generator.
        The PathologyPropertyWidgets are QObject-children of this object.
        The widgets are parent-less.
        The list can later be found in objects. Only the PathologyPropertyWidgets
        can be found in widgets.
      */
    QList<QObject*> createWidgets(Pathology::Entity entity);

    /**
      Adds the current list of widgets to a form layout
      */
    void addWidgetsToLayout(QFormLayout* layout);

    /**
      Deletes all objects
      */
    void clear();

    PathologyPropertyWidget* propertyWidget(const QString& property) const;

    Pathology::Entity m_entity;
    QList<QObject*>   m_objects;
    QList<PathologyPropertyWidget*> m_widgets;
    QHash<QString, PathologyPropertyWidget*> m_hash;

private:

    PathologyPropertyWidget* create(PathologyPropertyInfo::Property property);
};

#endif // PATHOLOGYWIDGETGENERATOR_H
