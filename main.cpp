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
#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QIcon>
#include <QMessageBox>
#include <QRegExp>
#include <QVariant>
#include <QUrl>

// Local includes

#include "analysisgenerator.h"
#include "csvconverter.h"
#include "databaseparameters.h"
#include "ihcscore.h"
#include <iostream>
#include "mainwindow.h"
#include "patientmanager.h"
#include "diseasehistory.h"
#include "reportwindow.h"
#include "historyvalidator.h"

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

    QCoreApplication::setOrganizationName("Innere Klinik (Tumorforschung)");
    QCoreApplication::setApplicationName("Tumorprofil");

    //if (!QIcon::hasThemeIcon("document-open")) {
        //If there is no default working icon theme then we should
        //use an icon theme that we provide via a .qrc file
        //This case happens under Windows and Mac OS X
        //This does not happen under GNOME or KDE
        QIcon::setThemeName("silk");
    //}

    qDebug() << QDir::currentPath();
    QFileInfo info("tumorprofil.db");
    QString dbFile;
    if (info.exists())
    {
        dbFile = info.filePath();
        qDebug() << "Using local database" << dbFile;
    }
    else
    {
        dbFile = "//ikt-hpstorage/wiesweg_DB/tumorprofil.db";
        qDebug() << "Using main database" << dbFile;
    }

    DatabaseParameters params =
            DatabaseParameters::parametersForSQLite(dbFile);

    if (!PatientManager::instance()->initialize(params))
    {
        return 1;
    }
    PatientManager::instance()->readDatabase();

    //CSVConverter::execute();
    //return 0;
    //ReportWindow report;
    //report.showMaximized();
    /*TNM tnm("cT3 Nx M1b (PUL BRA)");
    qDebug() << tnm.mstatus(TNM::AssumeM0IfMissing) << tnm.mstatus(TNM::AssumeMxIfMissing);
    return 0;*/
    AnalysisGenerator generator;
    generator.fishRatioListe();
    return 0;
    /*HistoryValidator validator;
    validator.validatePAC();
    return 0;*/

    MainWindow w;
    w.show();

    //DiseaseHistory::test();
    return a.exec();
}
