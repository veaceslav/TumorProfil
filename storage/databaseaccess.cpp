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

class DatabaseAccessStaticPriv
{
public:

    DatabaseAccessStaticPriv()
        : backend(0), db(0),
          initializing(false)
    {}
    ~DatabaseAccessStaticPriv() {};

    DatabaseCoreBackend* backend;
    PatientDB*          db;
    DatabaseParameters  parameters;
    DatabaseLocking     lock;
    QString             lastError;

    bool                initializing;
};

class DatabaseAccessMutexLocker : public QMutexLocker
{
public:

    DatabaseAccessMutexLocker(DatabaseAccessStaticPriv* d)
        : QMutexLocker(&d->lock.mutex), d(d)
    {
        d->lock.lockCount++;
    }

    ~DatabaseAccessMutexLocker()
    {
        d->lock.lockCount--;
    }

    DatabaseAccessStaticPriv* const d;
};

DatabaseAccessStaticPriv* DatabaseAccess::d = 0;

DatabaseAccess::DatabaseAccess()
{
    Q_ASSERT(d/*You will want to call setParameters before constructing DatabaseAccess*/);
    d->lock.mutex.lock();
    d->lock.lockCount++;

    if (!d->backend->isOpen() && !d->initializing)
    {
        // avoid endless loops
        d->initializing = true;

        d->backend->open(d->parameters);

        d->initializing = false;
    }
}

DatabaseAccess::~DatabaseAccess()
{
    d->lock.lockCount--;
    d->lock.mutex.unlock();
}

DatabaseAccess::DatabaseAccess(bool)
{
    // private constructor, when mutex is locked and
    // backend should not be checked
    d->lock.mutex.lock();
    d->lock.lockCount++;
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
    if (d)
    {
        return d->parameters;
    }

    return DatabaseParameters();
}

bool DatabaseAccess::isInitialized()
{
    return d;
}

void DatabaseAccess::initDatabaseErrorHandler(DatabaseErrorHandler* errorhandler)
{
    if (!d)
    {
        d = new DatabaseAccessStaticPriv();
    }

    //DatabaseErrorHandler *errorhandler = new DatabaseGUIErrorHandler(d->parameters);
    d->backend->setDatabaseErrorHandler(errorhandler);
}

void DatabaseAccess::setParameters(const DatabaseParameters& parameters)
{
    if (!d)
    {
        d = new DatabaseAccessStaticPriv();
    }

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

    if (!drivers.contains("QSQLITE"))
    {
        qWarning() << "No SQLite3 driver available. List of QSqlDatabase drivers: " << drivers;
        d->lastError = QObject::tr("The driver \"SQLITE\" for SQLite3 databases is not available.\n"
                            "digiKam depends on the drivers provided by the SQL module of Qt4.");
        return false;
    }

    // create an object with private shortcut constructor
    DatabaseAccess access(false);

    if (!d->backend)
    {
        qWarning() << "No database backend available in checkReadyForUse. "
                   "Did you call setParameters before?";
        return false;
    }

    if (d->backend->isReady())
    {
        return true;
    }

    if (!d->backend->isOpen())
    {
        if (!d->backend->open(d->parameters))
        {
            access.setLastError(QObject::tr("Error opening database backend.\n ")
                                + d->backend->lastError());
            return false;
        }
    }

    // avoid endless loops (if called methods create new DatabaseAccess objects)
    d->initializing = true;

    // update schema
    SchemaUpdater updater(&access);
    updater.setObserver(observer);

    if (!d->backend->initSchema(&updater))
    {
        d->initializing = false;
        return false;
    }

    d->initializing = false;

    return d->backend->isReady();
}

QString DatabaseAccess::lastError()
{
    return d->lastError;
}

void DatabaseAccess::setLastError(const QString& error)
{
    d->lastError = error;
}

void DatabaseAccess::cleanUpDatabase()
{
    if (d)
    {
        DatabaseAccessMutexLocker locker(d);
        d->backend->close();
        delete d->db;
        delete d->backend;
    }

    delete d;
    d = 0;
}

// --------

DatabaseAccessUnlock::DatabaseAccessUnlock()
{
    // acquire lock
    DatabaseAccess::d->lock.mutex.lock();
    // store lock count
    count = DatabaseAccess::d->lock.lockCount;
    // set lock count to 0
    DatabaseAccess::d->lock.lockCount = 0;

    // unlock
    for (int i=0; i<count; ++i)
    {
        DatabaseAccess::d->lock.mutex.unlock();
    }

    // drop lock acquired in first line. Mutex is now free.
    DatabaseAccess::d->lock.mutex.unlock();
}

DatabaseAccessUnlock::DatabaseAccessUnlock(DatabaseAccess*)
{
    // With the passed pointer, we have assured that the mutex is acquired
    // Store lock count
    count = DatabaseAccess::d->lock.lockCount;
    // set lock count to 0
    DatabaseAccess::d->lock.lockCount = 0;

    // unlock
    for (int i=0; i<count; ++i)
    {
        DatabaseAccess::d->lock.mutex.unlock();
    }

    // Mutex is now free
}

DatabaseAccessUnlock::~DatabaseAccessUnlock()
{
    // lock as often as it was locked before
    for (int i=0; i<count; ++i)
    {
        DatabaseAccess::d->lock.mutex.lock();
    }

    // update lock count
    DatabaseAccess::d->lock.lockCount += count;
}

