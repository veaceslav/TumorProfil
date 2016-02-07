/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 21.06.2015
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

#ifndef MODELFILTERLINEEDIT_H
#define MODELFILTERLINEEDIT_H

#include <QLineEdit>

class QAbstractItemView;

class ModelFilterLineEdit : public QLineEdit
{
    Q_OBJECT

public:

    /// The view's model() must be a QSortFilterProxyModel
    ModelFilterLineEdit(QAbstractItemView* view);

    virtual void keyPressEvent(QKeyEvent *e);

signals:

    /// Indicates the user selected the given index on the view
    void selected(const QModelIndex& index);

protected:

    QAbstractItemView*     view;
};

#endif // MODELFILTERLINEEDIT_H
