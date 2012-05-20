/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 07.05.2012
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

#ifndef PATHOLOGYMETADATAWIDGET_H
#define PATHOLOGYMETADATAWIDGET_H

#include <QWidget>

#include "pathology.h"

class PathologyMetadataWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PathologyMetadataWidget(QWidget *parent = 0);
    ~PathologyMetadataWidget();

    void saveValues(Pathology& path);
    void reset();
    
signals:

public slots:

    void readValues(const Pathology& path);

private:

    class PathologyMetadataWidgetPriv;
    PathologyMetadataWidgetPriv* const d;
    
};

#endif // PATHOLOGYMETADATAWIDGET_H
