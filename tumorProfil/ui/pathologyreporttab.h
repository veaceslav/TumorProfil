/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 03.08.2015
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

#ifndef PATHOLOGYREPORTTAB_H
#define PATHOLOGYREPORTTAB_H

#include <QWidget>

#include "pathology.h"

class PathologyReportTab : public QWidget
{
    Q_OBJECT
public:
    explicit PathologyReportTab(QWidget *parent = 0);
    ~PathologyReportTab();

public slots:

    void setPathologies(const QList<Pathology>& pathologies);

protected slots:

    void reportSelected(int index);

private:

    class PathologyReportTabPriv;
    PathologyReportTabPriv* const d;
};

#endif // PATHOLOGYREPORTTAB_H
