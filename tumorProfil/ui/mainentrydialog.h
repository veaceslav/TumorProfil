/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 2017-05-09
 * Description : Dialog to prompt users about main action
 *
 * Copyright (C) 2010-2012, 2017 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2013-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef MAINENTRYDIALOG_H
#define MAINENTRYDIALOG_H

// Qt includes

#include <QDialog>

class QAbstractButton;

class MainEntryDialog : public QDialog
{
    Q_OBJECT

public:

    enum Action
    {
        EditWindow,
        HistoryWindow,
        HistoryWindowReadOnly,
        ReportWindow,
        Cancel
    };

    static void executeAction(MainEntryDialog::Action action);

    explicit MainEntryDialog(QWidget* const parent = 0);
    ~MainEntryDialog();
    void setEnabled(Action action, bool enabled);
    void setEnabledFromAccessRights();

    Action action() const;

private Q_SLOTS:

    void slotButtonClicked(int id);

private:

    class Private;
    Private* const d;
};

#endif // MAINENTRYDIALOG_H
