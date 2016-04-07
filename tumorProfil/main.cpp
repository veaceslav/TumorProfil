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

#include <QApplication>
#include <QDir>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QIcon>
#include <QMessageBox>
#include <QRegExp>
#include <QTextStream>
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
#include "pathologyparser.h"
#include "pathologypropertiestableview.h"
#include "settings/encryptionsettings.h"
#include "settings/databasesettings.h"
#include "settings/mainsettings.h"
#include "encryption/authenticationwindow.h"
#include "encryption/userinformation.h"

/*
void myMsgHandler(QtMsgType, const char * text)
{
    QMessageBox::information(0, QString(), text);
}
*/

void handleAuthentication()
{
    UserInformation::instance()->logIn();
}

void checkDbConnection()
{
    DatabaseParameters params;
    params.readFromConfig();

    if(params.isMySQL()){
        handleAuthentication();
    } else {
        return;
    }
}

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
    a.setWindowIcon(QIcon::fromTheme("folder_table"));

    DatabaseParameters params;
    params.readFromConfig();

    checkDbConnection();

    if (!PatientManager::instance()->initialize(params))
    {
        return 1;
    }

    PatientManager::instance()->readDatabase();


    /*
    AnalysisGenerator generator;
    generator.ros1Project();
    return 0;
    */

    /*
    PathologyParser parser;
    QList<PatientParseResults> results =
    parser.parseFile("/home/marcel/Dokumente/Tumorprofil/Pathobefunde als Text/Pathobefunde 25.6.15 - 27.8.15.txt");
    QFile file("/home/marcel/Dokumente/Tumorprofil/Pathobefunde als Text/nichterkannt.txt");
    file.open(QFile::WriteOnly | QFile::Truncate);
    QTextStream stream(&file);
    foreach (const PatientParseResults& result, results)
    {
        stream << result.unrecognizedText;
        stream << "\n\n";
    }

    return 0;
    */
    /*
    PathologyPropertiesTableModel model;
    PathologyPropertiesTableFilterModel filterModel;
    PathologyPropertiesTableView view;
    view.setModels(&model, &filterModel);
    model.setProperties(parser.results().first().properties);
    view.resize(600, 500);
    view.resizeColumnsToContents();
    view.show();
    return a.exec();
    */

    MainWindow w;
    w.setWindowIcon(QIcon::fromTheme("folder_table"));
    w.show();

    //DiseaseHistory::test();
    return a.exec();
}
