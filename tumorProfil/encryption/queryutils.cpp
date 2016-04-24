#include "queryutils.h"

#include <QTime>
#include <QMap>
#include <QVariant>
#include <QDebug>
#include <QCryptographicHash>
#include <QApplication>
#include <QMessageBox>
#include <QSqlQuery>

#include "TumorUsers/aesutils.h"
#include "databaseaccess.h"
#include "constants.h"
#include "authentication/userinformation.h"

#define SALT_SIZE  10

#define DATABASE_CONNECTION_NAME "TumorUserConnection"


QueryUtils::QueryUtils(QObject *parent) : QObject(parent)
{

}

QVector<QVector<QVariant> > QueryUtils::retrieveMasterKeys(qlonglong userId)
{
    QMap<QString, QVariant> bindValues;
    QVector<QVector<QVariant> > results;
    bindValues[QLatin1String(":userid")] = userId;

    QueryUtils::executeDirectSql(QLatin1String("SELECT * from MasterKeys where userid=:userid"),
                                                 bindValues,
                                                 results,
                                                 QString(DATABASE_CONNECTION_NAME));
    return results;
}

QVector<QVector<QVariant> > QueryUtils::retrieveUserEntry(const QString &userName)
{
    QMap<QString, QVariant> bindValues;
    QVector<QVector<QVariant> > results;
    bindValues[QLatin1String(":username")] = userName;

    QueryUtils::executeDirectSql(QLatin1String("SELECT * from Users where name=:username"),
                                                 bindValues,
                                                 results,
                                                 QString(DATABASE_CONNECTION_NAME));
    return results;
}

UserDetails QueryUtils::retrieveUser(QString name, QString password)
{

    UserDetails details;
    DatabaseParameters params;
    params.readFromConfig();

    if(!openConnection(params, QString(DATABASE_CONNECTION_NAME), QueryUtils::TUMORUSER))
    {
        return details;
    }

    QVector<QVector<QVariant> > data = retrieveUserEntry(name);

    if(data.isEmpty())
        return details;

    if(QueryUtils::verifyPassword(password, data))
    {
        details.userName = data.first().at(NAME_INDEX).toString();
        details.id = data.first().at(USERID_INDEX).toInt();
        QString aesFilling = data.first().at(AESFILLING_INDEX).toString();
        qDebug() << "Aes filling:" << aesFilling;
        QVector<QVector<QVariant> > keys = retrieveMasterKeys(data.first().at(USERID_INDEX).toInt());

        foreach(QVector<QVariant> key, keys)
        {
            QString decryptedKey = AesUtils::decryptMasterKey(password, aesFilling, key.at(KEY_CONTENT_INDEX).toString());
            details.decryptionKeys.insert(key.at(KEY_NAME_INDEX).toString(), decryptedKey);
            //qDebug() << key.at(KEY_CONTENT_INDEX).toString();
            qDebug() << "Added key " << key.at(KEY_NAME_INDEX).toString() << " " << decryptedKey;
        }
    }

    return details;
}

bool QueryUtils::verifyPassword(const QString &password , const QVector<QVector<QVariant> > &result)
{
    QString saltedPass(password + result.first().at(PASSWORD_SALT_INDEX).toString());
    QByteArray passHash = QCryptographicHash::hash(saltedPass.toLatin1(), QCryptographicHash::Sha256);

    QByteArray storedHash = result.first().at(PASSWORD_HASH_INDEX).toByteArray();

    if(passHash != storedHash)
    {
        qDebug() << "Authentication problem! Incorrect Password";
        return false;
    }
    else
    {
        return true;
    }
}

bool QueryUtils::openConnection(DatabaseParameters params, QString databaseID, DatabaseName databaseName)
{
    QSqlDatabase database;

    qApp->setOverrideCursor(Qt::WaitCursor);

    {

        database = QSqlDatabase::addDatabase(params.databaseType,
                                             databaseID);

        if(!database.isValid())
        {
            QMessageBox::critical(qApp->activeWindow(), tr("Database connection test"),
                                  tr("Database connection test was not successful. <p>Error was:" +
                                       database.lastError().text().toLatin1() +  "</p>") );

            return false;
        }

        database.setHostName(params.hostName);
        database.setPort(params.port);
        database.setUserName(params.userName);
        database.setUserName(UserInformation::instance()->username());
        database.setPassword(UserInformation::instance()->password());

        qApp->restoreOverrideCursor();

        switch(databaseName)
        {
        case TUMORPROFIL:
            database.setDatabaseName(params.databaseName);
            break;
        case TUMORUSER:
            database.setDatabaseName(params.databaseNameThumbnails);
            break;
        default:
            qDebug() << "Error: Query utils, unknown database name type";
            break;
        }

        bool result = database.open();

        if (!result)
        {
            QMessageBox::critical(qApp->activeWindow(), tr("Database connection test"),
                                  tr("Database connection test was not successful. <p>Error was:" +
                                       database.lastError().text().toLatin1() +  "</p>") );
            database.close();
            QSqlDatabase::removeDatabase(databaseID);

        }

        QSqlQuery* testQuery = new QSqlQuery(database);
        testQuery->prepare(QLatin1String("show tables"));

        result = testQuery->exec();

        if (!result)
        {
            QMessageBox::critical(qApp->activeWindow(), tr("Database connection test"),
                                  tr("Database connection test was not successful. <p>Error was:" +
                                       testQuery->lastError().text().toLatin1() +  "</p>") );
            database.close();
            QSqlDatabase::removeDatabase(databaseID);
        }
    }
    return true;
}

bool QueryUtils::closeConnection(QString databaseID)
{
    QSqlDatabase::removeDatabase(databaseID);
}


bool QueryUtils::executeDirectSql(QString queryString, QMap<QString,
                                  QVariant> bindValues, QVector<QVector<QVariant> >& results,
                                  QString databaseID)
{

    QSqlQuery* query = new QSqlQuery(QSqlDatabase::database(databaseID));
    query->prepare(queryString);

    for(QMap<QString, QVariant>::iterator it = bindValues.begin(); it !=bindValues.end(); ++it)
    {
        query->bindValue(it.key(),it.value());
    }

    int result = query->exec();



    if(result)
    {
        while(query->next()){
            QVector<QVariant> lst;
            int index = 0;
            QVariant v = query->value(index);
            while(v.isValid())
            {
                lst << v;
                v = query->value(++index);
            }
            results.append(lst);
        }
        return true;
    }
    else
    {
        qDebug() << "Error:" << query->lastError().text().toLatin1();
        QMessageBox::critical(qApp->activeWindow(),tr("Error Executing Query"),
                                tr("Error while executing query [") + queryString + tr("] Error: ") + query->lastError().text().toLatin1());
        return false;
    }
}

