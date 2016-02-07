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

#ifndef RAWTEXTSUMMARYPAGE_H
#define RAWTEXTSUMMARYPAGE_H

#include <QWizardPage>

class QListWidget;

class RawTextSummaryPage : public QWizardPage
{
public:

    RawTextSummaryPage();
    virtual void initializePage();
    virtual void cleanupPage();

protected:

    QListWidget*    listWidget;
};

#endif // RAWTEXTSUMMARYPAGE_H
