/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 04.04.2012
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

#ifndef CSVCONVERTER_H
#define CSVCONVERTER_H

#include <QString>

class CSVConverter
{
public:

    static void execute();

    CSVConverter();

    void convertColon(const QString& filename);
    void convertBGJ389(const QString& filename);
    void convertBestRx(const QString& filename);
};

#endif // CSVCONVERTER_H
