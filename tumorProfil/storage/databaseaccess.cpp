/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-18
 * Description : Database access wrapper.
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

#include "databaseaccess.h"

// Qt includes

#include <QDebug>
#include <QMutex>
#include <QSqlDatabase>

// Local includes

#include "databasecorebackend.h"
#include "patientdb.h"
#include "schemaupdater.h"

class DatabaseAccessPriv
{
public:

    DatabaseAccessPriv();
    ~DatabaseAccessPriv();

    DatabaseCoreBackend* backend;
    PatientDB*          db;
    DatabaseParameters  parameters;
    DatabaseLocking     lock;
    QString             lastError;

    bool                initializing;

    void checkBackend();
    void constructorLock();
};

class DatabaseAccessMutexLocker : public QMutexLocker
{
public:

    DatabaseAccessMutexLocker(DatabaseAccessPriv* d)
        : QMutexLocker(&d->lock.mutex), d(d)
    {
        d->lock.lockCount++;
    }

    ~DatabaseAccessMutexLocker()
    {
        d->lock.lockCount--;
    }

    DatabaseAccessPriv* const d;
};

DatabaseAccessPriv::DatabaseAccessPriv()
    : backend(0),
      db(0),
      initializing(false)
{
}

DatabaseAccessPriv::~DatabaseAccessPriv()
{
    DatabaseAccessMutexLocker locker(this);
    backend->close();
    delete db;
    delete backend;
}

void DatabaseAccessPriv::checkBackend()
{
    if (!backend->isOpen() && !initializing)
    {
        // avoid endless loops
        initializing = true;

        backend->open(parameters);

        initializing = false;
    }
}

void DatabaseAccessPriv::constructorLock()
{
    lock.mutex.lock();
    lock.lockCount++;
}

DatabaseAccessPriv* DatabaseAccess::mainAccess = 0;

DatabaseAccess::DatabaseAccess()
    : d(mainAccess)
{
    Q_ASSERT(d/*You will want to call setParameters before constructing DatabaseAccess*/);
    d->constructorLock();
    d->checkBackend();
}

DatabaseAccess::~DatabaseAccess()
{
    d->lock.lockCount--;
    d->lock.mutex.unlock();
}

DatabaseAccess::DatabaseAccess(DatabaseAccessPriv* d)
    : d(d)
{
    // private constructor, when mutex is locked and
    // backend should not be checked
    d->constructorLock();
}

PatientDB* DatabaseAccess::db() const
{
    return d->db;
}

DatabaseCoreBackend* DatabaseAccess::backend() const
{
    return d->backend;
}

DatabaseParameters DatabaseAccess::parameters()
{
    if (mainAccess)
    {
        return mainAccess->parameters;
    }

    return DatabaseParameters();
}

QString DatabaseAccess::lastError()
{
    return d->lastError;
}

void DatabaseAccess::setLastError(const QString& error)
{
    d->lastError = error;
}

/// --- Static methods ---

bool DatabaseAccess::isInitialized()
{
    return mainAccess;
}

void DatabaseAccess::initDatabaseErrorHandler(DatabaseErrorHandler* errorhandler)
{
    if (mainAccess)
    {
        //DatabaseErrorHandler *errorhandler = new DatabaseGUIErrorHandler(d->parameters);
        mainAccess->backend->setDatabaseErrorHandler(errorhandler);
    }

}

void DatabaseAccess::setParameters(const DatabaseParameters& parameters)
{
    if (!mainAccess)
    {
        mainAccess = new DatabaseAccessPriv();
    }

    performSetParameters(mainAccess, parameters);
}

void DatabaseAccess::performSetParameters(DatabaseAccessPriv* d, const DatabaseParameters& parameters)
{
    DatabaseAccessMutexLocker lock(d);

    if (d->parameters == parameters)
    {
        return;
    }

    if (d->backend && d->backend->isOpen())
    {
        d->backend->close();
    }

    // Kill the old database error handler
    if (d->backend)
    {
        d->backend->setDatabaseErrorHandler(0);
    }

    d->parameters = parameters;

    if (!d->backend || !d->backend->isCompatible(parameters))
    {
        delete d->db;
        delete d->backend;
        d->backend = new DatabaseCoreBackend("database-", &d->lock);
        d->db = new PatientDB(d->backend);
    }
}

bool DatabaseAccess::checkReadyForUse(InitializationObserver* observer)
{
    QStringList drivers = QSqlDatabase::drivers();

    if (!mainAccess)
    {
        qWarning() << "mainAccess is null. You must call setParameters before checkReadyForUse";
        return false;
    }

    if (!drivers.contains("QSQLITE"))
    {
        qWarning() << "No SQLite3 driver available. List of QSqlDatabase drivers: " << drivers;
        mainAccess->lastError = QObject::tr("The driver \"SQLITE\" for SQLite3 databases is not available.\n"
                            "digiKam depends on the drivers provided by the SQL module of Qt4.");
        return false;
    }

    // create an object with private shortcut constructor
    DatabaseAccess access(mainAccess);
    return performReadyCheck(access, observer);
}

bool DatabaseAccess::performReadyCheck(DatabaseAccess& access, InitializationObserver* observer)
{
    if (!access.d->backend)
    {
        qWarning() << "No database backend available in checkReadyForUse. "
                   "Did you call setParameters before?";
        return false;
    }

    if (access.d->backend->isReady())
    {
        return true;
    }

    if (!access.d->backend->isOpen())
    {
        if (!access.d->backend->open(access.d->parameters))
        {
            access.setLastError(QObject::tr("Error opening database backend.\n ")
                                + access.d->backend->lastError());
            return false;
        }
    }

    // avoid endless loops (if called methods create new DatabaseAccess objects)
    access.d->initializing = true;

    // update schema
    SchemaUpdater updater(&access);
    updater.setObserver(observer);

    if (!access.d->backend->initSchema(&updater))
    {
        access.d->initializing = false;
        return false;
    }

    access.d->initializing = false;

    return access.d->backend->isReady();
}

void DatabaseAccess::cleanUpDatabase()
{
    delete mainAccess;
    mainAccess = 0;
}

DatabaseAccess* DatabaseAccess::createExternalAccess(const DatabaseParameters& params, InitializationObserver* observer)
{
    DatabaseAccessPriv* d = new DatabaseAccessPriv;
    DatabaseAccess* access = new DatabaseAccess(d);
    performSetParameters(d, params);
    if (!performReadyCheck(*access, observer))
    {
        delete access;
        return 0;
    }
    return access;
}

// --------

DatabaseAccessUnlock::DatabaseAccessUnlock(DatabaseAccess* access)
    : access(access)
{
    // With the passed pointer, we have assured that the mutex is acquired
    // Store lock count
    count = access->d->lock.lockCount;
    // set lock count to 0
    access->d->lock.lockCount = 0;

    // unlock
    for (int i=0; i<count; ++i)
    {
        access->d->lock.mutex.unlock();
    }

    // Mutex is now free
}

DatabaseAccessUnlock::~DatabaseAccessUnlock()
{
    // lock as often as it was locked before
    for (int i=0; i<count; ++i)
    {
        access->d->lock.mutex.lock();
    }

    // update lock count
    access->d->lock.lockCount += count;
}

