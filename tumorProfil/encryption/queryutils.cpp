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
#include "databasecorebackend.h"
#include "authentication/userinformation.h"

#define SALT_SIZE  10

#define DATABASE_CONNECTION_NAME "TumorUserConnection"

QPointer<TumorQueryUtils> TumorQueryUtils::internalPtr = QPointer<TumorQueryUtils>();


class TumorQueryUtils::Private
{
public:
    Private()
        : usersAccess(0)
    {
    }
    DatabaseAccess *usersAccess;
};

TumorQueryUtils* TumorQueryUtils::instance()
{
    if(TumorQueryUtils::internalPtr.isNull())
        TumorQueryUtils::internalPtr = new TumorQueryUtils();

    return TumorQueryUtils::internalPtr;
}

TumorQueryUtils::TumorQueryUtils():
    d(new Private())
{
}

bool TumorQueryUtils::open(const DatabaseParameters &params)
{
    instance()->d->usersAccess = DatabaseAccess::createExternalAccess(params);
    return instance()->d->usersAccess;
}

AbstractQueryUtils::QueryState TumorQueryUtils::executeSql(const QString& queryString,
                                                           QMap<QString, QVariant> bindValues, QVariant &lastId)
{

    SqlQuery query = d->usersAccess->backend()->prepareQuery(queryString);

    for(QMap<QString, QVariant>::iterator it = bindValues.begin(); it !=bindValues.end(); ++it)
    {
        query.bindValue(it.key(),it.value());
    }

    if(d->usersAccess->backend()->exec(query))
    {
        lastId = query.lastInsertId();
        return AbstractQueryUtils::NoErrors;
    }
    else
    {
        return AbstractQueryUtils::SQLError;
    }
}

AbstractQueryUtils::QueryState TumorQueryUtils::executeDirectSql(const QString& queryString,
                                                                 QMap<QString, QVariant> bindValues,
                                                                 QVector<QVector<QVariant> > &results)
{
    SqlQuery query = d->usersAccess->backend()->prepareQuery(queryString);

    for(QMap<QString, QVariant>::iterator it = bindValues.begin(); it !=bindValues.end(); ++it)
    {
        query.bindValue(it.key(),it.value());
    }

    if(d->usersAccess->backend()->exec(query))
    {
        while(query.next()){
            QVector<QVariant> lst;
            int index = 0;
            QVariant v = query.value(index);
            while(v.isValid())
            {
                lst << v;
                v = query.value(++index);
            }
            results.append(lst);
        }
        return AbstractQueryUtils::NoErrors;
    }
    else
    {
        return AbstractQueryUtils::SQLError;
    }
}

QVector<QVector<QVariant> > TumorQueryUtils::retrieveUserEntry(const QString &userName)
{
    QMap<QString, QVariant> bindValues;
    QVector<QVector<QVariant> > results;
    bindValues[QLatin1String(":username")] = userName;

    TumorQueryUtils::executeDirectSql(QLatin1String("SELECT * from Users where name=:username"),
                                                 bindValues,
                                                 results);
    return results;
}

UserDetails TumorQueryUtils::retrieveUser(const QString& name, const QString& password)
{

    UserDetails details;

    QVector<QVector<QVariant> > data = retrieveUserEntry(name);

    if(data.isEmpty())
        return details;

    if(TumorQueryUtils::verifyPassword(password, data))
    {
        details.userName = data.first().at(NAME_INDEX).toString();
        details.id = data.first().at(USERID_INDEX).toInt();
        QString salt = data.first().at(PASSWORD_SALT_INDEX).toString();
        details.userSalt = salt;
        QVector<QVector<QVariant> > keys = retrieveMasterKeys(data.first().at(USERID_INDEX).toInt());

        foreach(QVector<QVariant> key, keys)
        {
            QString decryptedKey = AesUtils::decryptMasterKey(password, salt, key.at(KEY_CONTENT_INDEX).toString());
            details.decryptionKeys.insert(key.at(KEY_NAME_INDEX).toString(), decryptedKey);
        }
    }

    return details;
}

bool TumorQueryUtils::verifyPassword(const QString &password , const QVector<QVector<QVariant> > &result)
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

