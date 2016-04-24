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


TumorQueryUtils::TumorQueryUtils(QObject *parent) : QObject(parent)
{

}

QVector<QVector<QVariant> > TumorQueryUtils::retrieveMasterKeys(qlonglong userId, QString databaseID)
{
    QMap<QString, QVariant> bindValues;
    QVector<QVector<QVariant> > results;
    bindValues[QLatin1String(":userid")] = userId;

    TumorQueryUtils::executeDirectSql(QLatin1String("SELECT * from MasterKeys where userid=:userid"),
                                                 bindValues,
                                                 results,
                                                 databaseID);
    return results;
}

QVector<QVector<QVariant> > TumorQueryUtils::retrieveUserEntry(const QString &userName, QString databaseID)
{
    QMap<QString, QVariant> bindValues;
    QVector<QVector<QVariant> > results;
    bindValues[QLatin1String(":username")] = userName;

    TumorQueryUtils::executeDirectSql(QLatin1String("SELECT * from Users where name=:username"),
                                                 bindValues,
                                                 results,
                                                 databaseID);
    return results;
}

UserDetails TumorQueryUtils::retrieveUser(QString name, QString password)
{

    UserDetails details;
    DatabaseParameters params;
    params.readFromConfig();

    if(!openConnection(params, QString(DATABASE_CONNECTION_NAME), TumorQueryUtils::TUMORUSER))
    {
        return details;
    }

    QVector<QVector<QVariant> > data = retrieveUserEntry(name, QString(DATABASE_CONNECTION_NAME));

    if(data.isEmpty())
        return details;

    if(TumorQueryUtils::verifyPassword(password, data))
    {
        details.userName = data.first().at(NAME_INDEX).toString();
        details.id = data.first().at(USERID_INDEX).toInt();
        QString aesFilling = data.first().at(AESFILLING_INDEX).toString();
        qDebug() << "Aes filling:" << aesFilling;
        QVector<QVector<QVariant> > keys = retrieveMasterKeys(data.first().at(USERID_INDEX).toInt(),
                                                              DATABASE_CONNECTION_NAME);

        foreach(QVector<QVariant> key, keys)
        {
            QString decryptedKey = AesUtils::decryptMasterKey(password, aesFilling, key.at(KEY_CONTENT_INDEX).toString());
            details.decryptionKeys.insert(key.at(KEY_NAME_INDEX).toString(), decryptedKey);
            qDebug() << "Added key " << key.at(KEY_NAME_INDEX).toString() << " " << decryptedKey;
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

bool TumorQueryUtils::openConnection(DatabaseParameters params, QString databaseID, DatabaseName databaseName)
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

bool TumorQueryUtils::closeConnection(QString databaseID)
{
    QSqlDatabase::removeDatabase(databaseID);

    return true;
}

bool TumorQueryUtils::removeAllMasterKeys(int userid, QString databaseID)
{
    QMap<QString, QVariant> bindValues;
    QVector<QVector<QVariant> > results;
    bindValues[QLatin1String(":userid")] = userid;

    return TumorQueryUtils::executeDirectSql(QLatin1String("DELETE from MasterKeys where userid=:userid"),
                                                 bindValues,
                                                 results,
                                                 databaseID);
}

bool TumorQueryUtils::updateUserMasterKeys(int userId, QString userPassword, QString userAesFilling,
                                      QMap<QString,QString> userKeys, QString databaseID)
{
    removeAllMasterKeys(userId,databaseID);

    QMap<QString, QString>::iterator it;
    for(it=userKeys.begin(); it != userKeys.end(); ++it)
    {
        addMasterKey(it.key(), userId, userPassword, userAesFilling,
                                 it.value());
    }
    return true;
}

QString generateRandomString(int length)
{
    qsrand(time(NULL));
    QString base("0123456789ABCDEF");
    QString str;
    for(int i=0;i<length;i++){
        str += base.at(qrand() % base.length());
    }

    return str;

}
qlonglong TumorQueryUtils::addMasterKey(QString name, qlonglong userid, QString password,
                                        QString aesFilling,
                                        QString databaseID, QString masterKey)
{
    if(masterKey.isEmpty())
        masterKey = generateRandomString(AESKEY_LENGTH);

    qDebug() << "Helo";
    QString encodedKey = AesUtils::encryptMasterKey(password, aesFilling, masterKey);

    QString decoded = AesUtils::decryptMasterKey(password, aesFilling, encodedKey);

    qDebug() << "End";
    if(decoded.compare(masterKey) != 0)
        qDebug() << "Wrong encryption. Expected: " << masterKey << " Got: " << decoded;

    QMap<QString, QVariant> bindValues;
    bindValues[QLatin1String(":keyName")] = name;
    bindValues[QLatin1String(":userid")] = userid;
    bindValues[QLatin1String(":encryptedKey")] = encodedKey;

    QVector<QVector<QVariant> > results;
    executeDirectSql(QLatin1String("INSERT into MasterKeys(keyName, userid, encryptedKey)"
                                                               "VALUES(:keyName, :userid, :encryptedKey)"),
                                                 bindValues,
                                                 results,
                                                 databaseID);

    return 0;
}

bool TumorQueryUtils::executeDirectSql(QString queryString, QMap<QString,
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

