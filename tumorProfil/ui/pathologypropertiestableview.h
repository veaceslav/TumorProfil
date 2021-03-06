/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 15.06.2015
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

#ifndef PATHOLOGYPROPERTIESTABLEVIEW_H
#define PATHOLOGYPROPERTIESTABLEVIEW_H

#include <QTableView>

#include "pathologypropertiestablemodel.h"

class PathologyPropertiesTableView : public QTableView
{
    Q_OBJECT

public:
    PathologyPropertiesTableView();

    PathologyPropertiesTableModel* propertiesModel() const;
    PathologyPropertiesTableFilterModel* filterModel() const;
    virtual void setModels(PathologyPropertiesTableModel*model, PathologyPropertiesTableFilterModel* filterModel);
};

#endif // PATHOLOGYPROPERTIESTABLEVIEW_H
