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

#include "modelfilterlineedit.h"

#include <QAbstractItemView>
#include <QKeyEvent>
#include <QSortFilterProxyModel>

ModelFilterLineEdit::ModelFilterLineEdit(QAbstractItemView* view)
    : view(view)
{
    connect(this, SIGNAL(textChanged(QString)),
            view->model(), SLOT(setFilterFixedString(QString)));
    setPlaceholderText(tr("Suche Patient"));
}

void ModelFilterLineEdit::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape)
    {
        clear();
        return;
    }
    if (e->key() == Qt::Key_Return)
    {
        QModelIndex index = view->currentIndex();
        if (!index.isValid())
        {
            index = view->model()->index(0,0);
            view->setCurrentIndex(index);
        }
        emit selected(index);
        clear();
        view->scrollTo(index);
        return;
    }
    QLineEdit::keyPressEvent(e);
}
