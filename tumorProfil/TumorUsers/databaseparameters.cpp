/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-18
 * Description : Storage container for database connection parameters.
 *
 * Copyright (C) 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010 by Holger Foerster <hamsi2k at freenet dot de>
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
/*
#ifndef DATABASEPARAMETERS_DEBUG
#define DATABASEPARAMETERS_DEBUG
#endif
*/

#include "databaseparameters.h"

// Qt includes

#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QUrlQuery>
#include <QSettings>



DatabaseParameters::DatabaseParameters()
    : port(-1)
{
}

DatabaseParameters::DatabaseParameters(const QString& type,
                                       const QString& databaseName,
                                       const QString& connectOptions,
                                       const QString& hostName,
                                       int port,
                                       const QString& userName,
                                       const QString& password,
                                       const QString& databaseNameThumbnails)
    : databaseType(type), databaseName(databaseName),
      connectOptions(connectOptions), hostName(hostName),
      port(port), userName(userName),
      password(password), databaseNameThumbnails(databaseNameThumbnails)
{
}

DatabaseParameters::DatabaseParameters(const QUrl& url)
    : port(-1)
{
    QUrlQuery urlQuery(url.query());
    databaseType   = urlQuery.queryItemValue("databaseType");
    databaseName   = urlQuery.queryItemValue("databaseName");
    databaseNameThumbnails   = urlQuery.queryItemValue("databaseNameThumbnails");
    connectOptions = urlQuery.queryItemValue("connectOptions");
    hostName       = urlQuery.queryItemValue("hostName");
    QString queryPort = urlQuery.queryItemValue("port");

    if (!queryPort.isNull())
    {
        port = queryPort.toInt();
    }

    userName       = urlQuery.queryItemValue("userName");
    password       = urlQuery.queryItemValue("password");
}

bool DatabaseParameters::operator==(const DatabaseParameters& other) const
{
    return databaseType   == other.databaseType &&
           databaseName   == other.databaseName &&
           databaseNameThumbnails == other.databaseNameThumbnails &&
           connectOptions == other.connectOptions &&
           hostName       == other.hostName &&
           port           == other.port &&
           userName       == other.userName &&
           password       == other.password;
}

bool DatabaseParameters::operator!=(const DatabaseParameters& other) const
{
    return !operator==(other);
}

bool DatabaseParameters::isValid() const
{
    if (isSQLite())
    {
        return !databaseName.isEmpty();
    }

    if(isMySQL())
    {
        bool check = true;

        check &= !databaseName.isEmpty();
        check &= (port != 0);
        check &= !hostName.isEmpty();
        check &= !userName.isEmpty();
        check &= !password.isEmpty();

        return check;
    }

    return false;
}

bool DatabaseParameters::isSQLite() const
{
    return databaseType == "QSQLITE";
}

bool DatabaseParameters::isMySQL() const
{
    return databaseType == "QMYSQL";
}

QString DatabaseParameters::SQLiteDatabaseType()
{
    return "QSQLITE";
}

QString DatabaseParameters::MySQLDatabaseType()
{
    return "QMYSQL";
}

QString DatabaseParameters::SQLiteDatabaseFile() const
{
    if (isSQLite())
    {
        return databaseName;
    }

    return QString();
}

QByteArray DatabaseParameters::hash() const
{
    QCryptographicHash md5(QCryptographicHash::Md5);
    md5.addData(databaseType.toUtf8());
    md5.addData(databaseName.toUtf8());
    md5.addData(connectOptions.toUtf8());
    md5.addData(hostName.toUtf8());
    md5.addData((const char*)&port, sizeof(int));
    md5.addData(userName.toUtf8());
    md5.addData(password.toUtf8());
    return md5.result();
}

/*
DatabaseParameters DatabaseParameters::parametersFromConfig(KSharedConfig::Ptr config, const QString& configGroup)
{
    DatabaseParameters parameters;
    parameters.readFromConfig(config, configGroup);
    return parameters;
}

*/
//void DatabaseParameters::readFromConfig(const QString& programName, const QString& organizationName)
//{
//    QSettings qs(organizationName, programName);

//    qs.beginGroup(QLatin1String(configGroupDatabase));

//    databaseType = qs.value(configDatabaseType,DatabaseParameters::SQLiteDatabaseType()).toString();
//    databaseName            = qs.value(configDatabaseName, QString()).toString();
//    databaseNameThumbnails  = qs.value(configDatabaseNameThumbnails, QString()).toString();
//    hostName                = qs.value(configDatabaseHostName, QString()).toString();
//    userName                = qs.value(configDatabaseUsername, QString()).toString();
//    password                = qs.value(configDatabasePassword, QString()).toString();
//    connectOptions          = qs.value(configDatabaseConnectOptions, QString()).toString();
//    port                    = qs.value(configDatabasePort, -1).toInt();

//    sqliteDatabasePath      = qs.value(configDatabaseFilePathEntry, QDir::current().absolutePath()).toString();
//    sqliteDatabaseName      = qs.value(configSqliteDatabaseName, QString()).toString();
//    sqliteUserDatabaseName  = qs.value(configSqliteUserDatabaseName, QString()).toString();

//    if (isSQLite() && sqliteDatabaseName.isEmpty() && sqliteUserDatabaseName.isEmpty())
//    {
//        if(sqliteDatabasePath.isEmpty())
//            sqliteDatabasePath = QDir::current().absolutePath();

//        QString orgName = sqliteDatabasePath;
//        setDatabasePath(orgName);
//        setThumbsDatabasePath(orgName);
//    }
//}

//void DatabaseParameters::writeToConfig(const QString &programName, const QString& organizationName) const
//{
//    QSettings qs(organizationName, programName);

//    qs.beginGroup(QLatin1String(configGroupDatabase));

//    qs.setValue(configDatabaseType, databaseType);
//    qs.setValue(configDatabaseName, databaseName);
//    qs.setValue(configDatabaseNameThumbnails, databaseNameThumbnails);
//    qs.setValue(configDatabaseHostName, hostName);
//    qs.setValue(configDatabaseUsername, userName);
//    qs.setValue(configDatabasePassword, password);
//    qs.setValue(configDatabaseConnectOptions, connectOptions);
//    qs.setValue(configDatabasePort, port);

//    // SQLITE values
//    qs.setValue(configDatabaseFilePathEntry, sqliteDatabasePath);
//    qs.setValue(configSqliteDatabaseName, sqliteDatabaseName);
//    qs.setValue(configSqliteUserDatabaseName,sqliteUserDatabaseName);

//}


void DatabaseParameters::setDatabasePath(const QString& folderOrFileOrName)
{
    if (isSQLite())
    {
        sqliteDatabaseName = databaseFileSQLite(folderOrFileOrName);
    }
    else
    {
        databaseName = folderOrFileOrName;
    }
}

void DatabaseParameters::setThumbsDatabasePath(const QString& folderOrFileOrName)
{
    if (isSQLite())
    {
        sqliteUserDatabaseName = thumbnailDatabaseFileSQLite(folderOrFileOrName);
    }
    else
    {
        databaseNameThumbnails = folderOrFileOrName;
    }
}

QString DatabaseParameters::databaseFileSQLite(const QString& folderOrFile)
{
//    QFileInfo fileInfo(folderOrFile);

//    if (fileInfo.isDir())
//    {
//        return QDir::cleanPath(fileInfo.filePath() + QDir::separator() + tumorprofildb);
//    }

    return QDir::cleanPath(folderOrFile);
}

QString DatabaseParameters::thumbnailDatabaseFileSQLite(const QString& folderOrFile)
{
//    QFileInfo fileInfo(folderOrFile);

//    if (fileInfo.isDir())
//    {
//        return QDir::cleanPath(fileInfo.filePath() + QDir::separator() + tumorprofilusersdb);
//    }

    return QDir::cleanPath(folderOrFile);
}

QString DatabaseParameters::getDatabaseNameOrDir() const
{
    if (isSQLite())
    {
        return sqliteDatabaseName;
    }

    return databaseName;
}

QString DatabaseParameters::getUsersDatabaseNameOrDir() const
{
    if (isSQLite())
    {
        return sqliteUserDatabaseName;
    }

    return databaseNameThumbnails;
}

QString DatabaseParameters::databaseDirectorySQLite(const QString& path)
{
//    if (path.endsWith(tumorprofildb))
//    {
//        QString chopped(path);
//        chopped.chop(QString(tumorprofildb).length());
//        return chopped;
//    }

    return path;
}

QString DatabaseParameters::thumbnailDatabaseDirectorySQLite(const QString& path)
{
//    if (path.endsWith(tumorprofilusersdb))
//    {
//        QString chopped(path);
//        chopped.chop(QString(tumorprofilusersdb).length());
//        return chopped;
//    }

    return path;
}

DatabaseParameters DatabaseParameters::defaultParameters(const QString databaseType)
{
    DatabaseParameters parameters;

    // only the database name is needed
    DatabaseConfigElement config = DatabaseConfigElement::element(databaseType);

    parameters.databaseType     = databaseType;
    parameters.databaseName     = config.databaseName;
    parameters.hostName         = config.hostName;
    parameters.userName         = config.userName;
    parameters.password         = config.password;
    parameters.port             = config.port.toInt();


    qDebug() << "ConnectOptions "<< parameters.connectOptions;
    return parameters;
}

DatabaseParameters DatabaseParameters::thumbnailParameters() const
{
    DatabaseParameters params = *this;
    params.databaseName = databaseNameThumbnails;
    return params;
}

//DatabaseParameters DatabaseParameters::parametersForSQLite(const QString& databaseFile)
//{
//    // only the database name is needed
//    DatabaseParameters params("QSQLITE", databaseFile);
//    params.setDatabasePath(databaseFile);
//    params.setThumbsDatabasePath(params.getDatabaseNameOrDir());
//    return params;
//}


void DatabaseParameters::insertInUrl(QUrl& url) const
{
    removeFromUrl(url);

    QUrlQuery urlQuery(url.query());
    urlQuery.addQueryItem("databaseType", databaseType);
    urlQuery.addQueryItem("databaseName", databaseName);

    if (!connectOptions.isNull())
    {
        urlQuery.addQueryItem("connectOptions", connectOptions);
    }

    if (!hostName.isNull())
    {
        urlQuery.addQueryItem("hostName", hostName);
    }

    if (port != -1)
    {
        urlQuery.addQueryItem("port", QString::number(port));
    }

    if (!userName.isNull())
    {
        urlQuery.addQueryItem("userName", userName);
    }

    if (!password.isNull())
    {
        urlQuery.addQueryItem("password", password);
    }

    url.setQuery(urlQuery);
}

void DatabaseParameters::removeFromUrl(QUrl& url)
{
    QUrlQuery urlQuery(url.query());
    urlQuery.removeQueryItem("databaseType");
    urlQuery.removeQueryItem("databaseName");
    urlQuery.removeQueryItem("connectOptions");
    urlQuery.removeQueryItem("hostName");
    urlQuery.removeQueryItem("port");
    urlQuery.removeQueryItem("internalServer");
    urlQuery.removeQueryItem("userName");
    urlQuery.removeQueryItem("password");

    url.setQuery(urlQuery);
}

QDebug operator<<(QDebug dbg, const DatabaseParameters& p)
{
    dbg.nospace() << "DatabaseParameters: [ Type "
                  << p.databaseType << ", ";
    dbg.nospace() << "Name "
                  << p.databaseName << " ";
    dbg.nospace() << "(Thumbnails Name "
                  << p.databaseNameThumbnails << "); ";

    if (!p.connectOptions.isEmpty())
        dbg.nospace() << "ConnectOptions: "
                      << p.connectOptions << ", ";

    if (!p.hostName.isEmpty())
        dbg.nospace() << "Host Name and Port: "
                      << p.hostName << " " << p.port << "; ";


    if (!p.userName.isEmpty())
        dbg.nospace() << "Username and Password: "
                      << p.userName << ", " << p.password;

    dbg.nospace() << "] ";

    return dbg.space();
}

