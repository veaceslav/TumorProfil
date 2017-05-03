/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-29
 * Description : Thumbnail database access wrapper.
 *
 * Copyright (C) 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DATABASEACCESS_H
#define DATABASEACCESS_H

// Local includes

#include "databaseparameters.h"
#include "databaseerrorhandler.h"
class QMutexLocker;

class DatabaseCoreBackend;
class InitializationObserver;
class PatientDB;
class DatabaseAccessPriv;

class DatabaseAccess
{
public:

    /** This class is written in analogy to DatabaseAccess
     *  (some features stripped off).
     *  For documentation, see databaseaccess.h */

    DatabaseAccess();
    ~DatabaseAccess();

    PatientDB* db() const;

    DatabaseCoreBackend* backend() const;
    QString lastError();
    static DatabaseParameters parameters();


    static void initDatabaseErrorHandler(DatabaseErrorHandler* errorhandler);
    static void setParameters(const DatabaseParameters& parameters);
    static bool checkReadyForUse(InitializationObserver* observer);
    static bool isInitialized();

    static void cleanUpDatabase();

    /**
      * Set the "last error" message. This method is not for public use.
      */
    void setLastError(const QString& error);

    /**
     * Create a DatabaseAccess for a database which is not the main database.
     * The makes no assumption on the underlying db schema. The backend will be open.
     * The second assumes a PatientDB-supported schema and schema-updates, if necessary. The backend will be ready.
     * You must delete the DatabaseAccess object which you receive.
     */
    static DatabaseAccess* createExternalAccess(const DatabaseParameters& params);
    static DatabaseAccess* createExternalPatientDBAccess(const DatabaseParameters& params, InitializationObserver* observer);

private:

    DatabaseAccess(DatabaseAccessPriv* d);
    friend class DatabaseAccessUnlock;
    static void performSetParameters(DatabaseAccessPriv* d, const DatabaseParameters& parameters);
    bool performReadyCheck();
    bool performSchemaUpdate(InitializationObserver* observer);

    DatabaseAccessPriv* d;
    static DatabaseAccessPriv* mainAccess;
};

class DatabaseAccessUnlock
{
public:

    /** Acquire an object of this class if you want to assure
     *  that the DatabaseAccess is _not_ held during the lifetime of the object.
     *  At creation, the lock is obtained shortly, then all locks are released.
     *  At destruction, all locks are acquired again.
     *  If you need to access any locked structures during lifetime, acquire a new
     *  DatabaseAccess.
     */
    DatabaseAccessUnlock(DatabaseAccess* access);
    ~DatabaseAccessUnlock();

private:

    DatabaseAccess* access;
    int count;
};


#endif // DATABASEACCESS_H
