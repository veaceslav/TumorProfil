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

#ifndef TNMWIDGET_H
#define TNMWIDGET_H

// Qt includes

#include <QWidget>

// Local includes

#include "tnm.h"

class TNMWidget : public QWidget
{
    Q_OBJECT
public:

    explicit TNMWidget(QWidget *parent = 0);
    ~TNMWidget();

    TNM currentTNM() const;

signals:

public slots:

    void setTNM(const TNM& tnm);

protected slots:

    void tnmEdited(const QString&);
    void updateResultDisplay();

private:

    class TNMWidgetPriv;
    TNMWidgetPriv* const d;

};

#endif // TNMWIDGET_H
