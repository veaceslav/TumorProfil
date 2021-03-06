/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-18
 * Description : Storage container for database connection parameters.
 *
 * Copyright (C) 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DATABASEPARAMETERS_H
#define DATABASEPARAMETERS_H

// Qt includes

#include <QString>
#include <QtGlobal>
#include <QUrl>

// Local includes

#include "databaseconfigelement.h"
//#include "constants.h"

class DatabaseParameters
{

public:

    /**
      * This class encapsulates all parameters needed to establish
      * a connection to a database (inspired by the API of QT SQL of Qt4).
      * The values can be read from and written to a KUrl.
      */

    DatabaseParameters(const QString& type,
                       const QString& databaseName,
                       const QString& connectOptions = QString(),
                       const QString& hostName = QString(),
                       int   port = -1,
                       const QString& userName = QString(),
                       const QString& password = QString(),
                       const QString& databaseNameUsers = QString());

    DatabaseParameters(const QUrl& url);
    DatabaseParameters();

    // MYSQL Parameters
    QString databaseType;
    QString databaseName;
    QString connectOptions;
    QString hostName;
    int     port;
    QString userName;
    QString password;

    QString databaseNameUsers;

    // SQLITE parameters
    QString sqliteUserDatabaseName;
    QString sqliteDatabaseName;
    QString sqliteDatabasePath;

    void insertInUrl(QUrl& url) const;
    bool operator==(const DatabaseParameters& other) const;
    bool operator!=(const DatabaseParameters& other) const;

    /** Performs basic checks that the parameters are not empty and have the information
     *  required for the databaseType.
     */
    bool isValid() const;

    bool isSQLite() const;
    bool isMySQL() const;
    QString SQLiteDatabaseFile() const;

    /**
     *  Returns the databaseType designating the said database.
     *  If you have a DatabaseParameters object already, you can use isSQLite() as well.
     *  These strings are identical to the driver identifiers in the Qt SQL module.
     */
    static QString SQLiteDatabaseType();
    static QString MySQLDatabaseType();

    /**
     * Creates a unique hash of the values stored in this object.
     */
    QByteArray hash() const;

    /** Return a set of default parameters for the give type */
    static DatabaseParameters defaultParameters(const QString databaseType);


    /**
     * In case of SQLite, the databaseName typically is a file.
     * With getDatabasePath, you can get the folder.
     */
    QString getDatabaseNameOrDir() const;
    QString getUsersDatabaseNameOrDir() const;

    /**
     * In case of SQLite, the databaseName typically is a file.
     * Use this method if you set a file or a folder.
     * For non-SQLite, this simply sets the database name.
     */
    void setDatabasePath(const QString& folderOrFileOrName);
    void setThumbsDatabasePath(const QString& folderOrFileOrName);

    static QString databaseFileSQLite(const QString& folderOrFile);
    static QString thumbnailDatabaseFileSQLite(const QString& folderOrFile);
    static QString databaseDirectorySQLite(const QString& path);
    static QString thumbnailDatabaseDirectorySQLite(const QString& path);

    /** Replaces databaseName with databaseNameThumbnails. */
    DatabaseParameters thumbnailParameters() const;


    static void removeFromUrl(QUrl& url);
};

QDebug operator<<(QDebug dbg, const DatabaseParameters& t);


#endif // DATABASEPARAMETERS_H
