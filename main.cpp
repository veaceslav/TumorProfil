/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 2012-01-22
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

// Qt includes

#include <QtGui/QApplication>
#include <QDebug>
#include <QFileInfo>
#include <QIcon>
#include <QMessageBox>
#include <QVariant>
#include <QUrl>

// Local includes

#include "csvconverter.h"
#include "databaseparameters.h"
#include "mainwindow.h"
#include "patientmanager.h"

/*
void myMsgHandler(QtMsgType, const char * text)
{
    QMessageBox::information(0, QString(), text);
}
*/

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //qInstallMsgHandler(myMsgHandler);

    if (!QIcon::hasThemeIcon("document-open")) {
        //If there is no default working icon theme then we should
        //use an icon theme that we provide via a .qrc file
        //This case happens under Windows and Mac OS X
        //This does not happen under GNOME or KDE
        QIcon::setThemeName("silk");
    }

    QFileInfo info("tumorprofil.db");
    QString dbFile;
    if (info.exists())
    {
        dbFile = info.filePath();
    }
    else
    {
        dbFile = "//ikt-hpstorage/wiesweg_DB/tumorprofil.db";
    }
    qDebug() << "Using database" << dbFile;

    DatabaseParameters params =
            DatabaseParameters::parametersForSQLite(dbFile);

    if (!PatientManager::instance()->initialize(params))
    {
        return 1;
    }
    PatientManager::instance()->readDatabase();

    //CSVConverter::execute();
    //return 0;

    MainWindow w;
    w.show();

    return a.exec();
}
