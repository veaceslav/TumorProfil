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

#ifndef RAWTEXTENTERPAGE_H
#define RAWTEXTENTERPAGE_H

#include <QTextEdit>
#include <QWizardPage>

class RawTextEnterPage : public QWizardPage
{
    Q_OBJECT

public:

    RawTextEnterPage();

protected slots:

    void copyClipboard();

protected:

    QTextEdit* textEdit;
};

#endif // RAWTEXTENTERPAGE_H
