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
#include <QCommandLineParser>
#include <QDir>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QFuture>
#include <QFutureWatcher>
#include <QIcon>
#include <QMessageBox>
#include <QProgressDialog>
#include <QRegExp>
#include <QtConcurrent/QtConcurrent>
#include <QTextStream>
#include <QVariant>
#include <QUrl>

// Local includes

#include "databaseaccess.h"
#include "analysisgenerator.h"
#include "csvconverter.h"
#include "databaseparameters.h"
#include "ihcscore.h"
#include <iostream>
#include "mainentrydialog.h"
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
#include "authentication//userinformation.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("Innere Klinik (Tumorforschung)");
    QCoreApplication::setApplicationName("Tumorprofil");

    QIcon::setThemeName("silk");
    app.setWindowIcon(QIcon::fromTheme("folder_table"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("Tumorprofil - Datenbankanwendung"));
    parser.addHelpOption();
    QCommandLineOption editOption("edit", QObject::tr("Öffne Fenster zur Befundeingabe"));
    parser.addOption(editOption);
    QCommandLineOption historyOption("history-edit", QObject::tr("Öffne Fenster zum Bearbeiten des Krankheitsverlaufs"));
    parser.addOption(historyOption);
    QCommandLineOption historyReadOption("history", QObject::tr("Öffne Fenster zum Einsehen des Krankheitsverlaufs"));
    parser.addOption(historyReadOption);
    QCommandLineOption reportOption("report", QObject::tr("Öffne Fenster zur Datenabfrage"));
    parser.addOption(reportOption);

    parser.process(app);

    DatabaseParameters params;
    params.readFromConfig();
    if(params.isMySQL())
    {
        if (!UserInformation::instance()->logIn())
        {
            return 1;
        }
    }
    else
    {
        DatabaseAccess::setParameters(params);
    }

    if (!PatientManager::instance()->initialize())
    {
        return 1;
    }

    {
        QFutureWatcher<void> watcher;
        QProgressDialog progressDialog;
        progressDialog.setLabelText(QObject::tr("Lade Datenbank"));
        QObject::connect(PatientManager::instance(), &PatientManager::progressStarted, &progressDialog, &QProgressDialog::setMaximum);
        QObject::connect(PatientManager::instance(), &PatientManager::progressValue, &progressDialog, &QProgressDialog::setValue);
        QObject::connect(&watcher, SIGNAL(finished()), &progressDialog, SLOT(accept()));
        QFuture<void> future = QtConcurrent::run(PatientManager::instance(), &PatientManager::readDatabase);
        watcher.setFuture(future);
        progressDialog.exec();
    }

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
    /*foreach (const Patient::Ptr& p, PatientManager::instance()->patients())
    {
        if (!p->hasDisease())
        {
            continue;
        }
        Disease& d = p->firstDisease();
        if (d.history.isEmpty())
        {
            continue;
        }
        if (d.history.testXmlEvent())
        {
            qDebug() << "Success" << p->id;
        }
        else
        {
            qDebug() << p->id << " " << p->firstName << " " << p->surname << " " << p->dateOfBirth;
            return 0;
        }
    }*/

    QPointer<MainEntryDialog> mainEntryDialog;
    if (parser.isSet(editOption))
    {
        MainEntryDialog::executeAction(MainEntryDialog::EditWindow);
    }
    else if (parser.isSet(reportOption))
    {
        MainEntryDialog::executeAction(MainEntryDialog::ReportWindow);
    }
    else if (parser.isSet(historyOption))
    {
        MainEntryDialog::executeAction(MainEntryDialog::HistoryWindow);
    }
    else if (parser.isSet(historyReadOption))
    {
        MainEntryDialog::executeAction(MainEntryDialog::HistoryWindowReadOnly);
    }
    else
    {
        mainEntryDialog = new MainEntryDialog;
        mainEntryDialog->show();
    }

    return app.exec();
}
