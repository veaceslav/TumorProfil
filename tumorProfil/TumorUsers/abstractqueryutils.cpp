#include "abstractqueryutils.h"

#include <QTime>
#include <QMap>
#include <QVariant>
#include <QDebug>
#include <QCryptographicHash>

#include "aesutils.h"
#include "time.h"

#define SALT_SIZE  10



AbstractQueryUtils::AbstractQueryUtils(QObject *parent) : QObject(parent)
{

}


UserDetails AbstractQueryUtils::addUser(QString name, AbstractQueryUtils::UserType userType, QString password)
{
   QMap<QString, QVariant> bindValues;


   QString salt = generateRandomString(SALT_SIZE);

   // Compute password
   QString saltedPass(password + salt);
   QByteArray passHash = QCryptographicHash::hash(saltedPass.toLatin1(), QCryptographicHash::Sha256);

   // add aes private key filling
   QString aesFilling = generateRandomString(AESKEY_LENGTH);


   bindValues[QLatin1String(":name")] = name;

   if(userType == AbstractQueryUtils::ADMIN)
       bindValues[QLatin1String(":usergroup")] = "ADMIN";
   else
       bindValues[QLatin1String(":usergroup")] = "USER";

   bindValues[QLatin1String(":passwordSalt")] = salt;
   bindValues[QLatin1String(":passwordHash")] = QVariant(passHash);
   bindValues[QLatin1String(":aesPrivateKeyFilling")] = aesFilling;

   QVariant id;
   executeSql(QLatin1String("INSERT into Users(name, usergroup, passwordSalt,"
                            " passwordHash, aesPrivateKeyFilling)"
                            "VALUES(:name, :usergroup, :passwordSalt, :passwordHash,"
                            ":aesPrivateKeyFilling)"),
              bindValues,
              id);


   qDebug() << "Id of inserted item: " << id;

    // According to MySql documentation, we add a user with wildcard "%", but we also need to add the same user
    // with localhost
   addMySqlUser(name,password);
   addMySqlUser(name,password, QLatin1String("localhost"));

   return UserDetails(id.toLongLong(),aesFilling);
}

UserDetails AbstractQueryUtils::editUser(QString name, AbstractQueryUtils::UserType userType, QString password, qlonglong userId)
{
    QMap<QString, QVariant> bindValues;


    QString salt = generateRandomString(SALT_SIZE);

    // Compute password
    QString saltedPass(password + salt);
    QByteArray passHash = QCryptographicHash::hash(saltedPass.toLatin1(), QCryptographicHash::Sha256);

    // add aes private key filling
    QString aesFilling = generateRandomString(AESKEY_LENGTH);


    bindValues[QLatin1String(":name")] = name;

    if(userType == AbstractQueryUtils::ADMIN)
        bindValues[QLatin1String(":usergroup")] = "ADMIN";
    else
        bindValues[QLatin1String(":usergroup")] = "USER";

    bindValues[QLatin1String(":passwordSalt")] = salt;
    bindValues[QLatin1String(":passwordHash")] = QVariant(passHash);
    bindValues[QLatin1String(":aesPrivateKeyFilling")] = aesFilling;
    bindValues[QLatin1String(":userid")] = userId;

    QVariant id;
    executeSql(QLatin1String("UPDATE Users"
                             " SET name= :name, usergroup= :usergroup, passwordSalt= :passwordSalt, "
                             " passwordHash= :passwordHash, aesPrivateKeyFilling= :aesPrivateKeyFilling"
                             " WHERE id = :userid"),
               bindValues,
               id);


    setMySqlPassword(name, password);
    qDebug() << "Id of inserted item: " << userId;

    return UserDetails(userId,aesFilling);
}

bool AbstractQueryUtils::updateUserMasterKeys(int userId, QString userPassword, QString userAesFilling,
                                      QMap<QString,QString> userKeys)
{
    AbstractQueryUtils::removeAllMasterKeys(userId);

    QMap<QString, QString>::iterator it;
    for(it=userKeys.begin(); it != userKeys.end(); ++it)
    {
        AbstractQueryUtils::addMasterKey(it.key(), userId, userPassword, userAesFilling,
                                 it.value());
    }

    return true;
}

bool AbstractQueryUtils::addMySqlUser(QString user, QString password, QString host)
{
    QMap<QString, QVariant> bindValues;
    QVector<QVector<QVariant> > results;

    QString query = QString("CREATE USER '%1'@'%2' IDENTIFIED BY '%3'").arg(user, host, password);
    executeDirectSql(query,
                     bindValues,
                     results);
    return true;
}

bool AbstractQueryUtils::deleteMySqlUser(QString user, QString host)
{
    QMap<QString, QVariant> bindValues;
    QVector<QVector<QVariant> > results;

    QString query = QString("DROP USER IF EXISTS '%1'@'%2'").arg(user, host);
    executeDirectSql(query,
                     bindValues,
                     results);

    return true;
}

bool AbstractQueryUtils::setMySqlPassword(QString user, QString password, QString host)
{
    QMap<QString, QVariant> bindValues;
    QVector<QVector<QVariant> > results;

    QString query = QString("SET PASSWORD FOR %1'@'%2' = PASSWORD('%3')").arg(user, host, password);
    executeDirectSql(query,
                     bindValues,
                     results);
    return true;
}

bool AbstractQueryUtils::grantMySqlPermissions(QString user, QString databaseName, QString userHost, QString tableName, QString type)
{
    QMap<QString, QVariant> bindValues;
    QVector<QVector<QVariant> > results;

    QString query = QString("GRANT %1 ON %2.%3 TO '%4'@'%5'").arg(type,
                                                                  databaseName,
                                                                  tableName,
                                                                  user,
                                                                  userHost);
    executeDirectSql(query,
                     bindValues,
                     results);
    return true;
}

QVector<QString> AbstractQueryUtils::getTumorProfilTables(QString tumorProfilDbName)
{
    QMap<QString, QVariant> bindValues;
    QVector<QVector<QVariant> > results;
    QVector<QString> result;

    QString query = QString("show tables from %1").arg(tumorProfilDbName);
    executeDirectSql(query,
                     bindValues,
                     results);

    for(QVector<QVariant> val : results)
    {
        result.append(val.first().toString());
    }

    return result;
}

bool AbstractQueryUtils::revokeAllPrivileges(QString user)
{
    QMap<QString, QVariant> bindValues;
    QVector<QVector<QVariant> > results;

    QString query = QString("REVOKE ALL PRIVILEGES, GRANT OPTION FROM %1").arg(user);
    executeDirectSql(query,
                     bindValues,
                     results);

    return true;
}

QString AbstractQueryUtils::generateRandomString(int length)
{
    qsrand(time(NULL));
    QString base("0123456789ABCDEF");
    QString str;
    for(int i=0;i<length;i++){
        str += base.at(qrand() % base.length());
    }

    return str;

}


qlonglong AbstractQueryUtils::addMasterKey(QString name, qlonglong userid, QString password, QString aesFilling, QString masterKey)
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

    QVariant id;
    executeSql(QLatin1String("INSERT into MasterKeys(keyName, userid, encryptedKey)"
                             "VALUES(:keyName, :userid, :encryptedKey)"),
               bindValues,
               id);

    return id.toLongLong();
}

QVector<QVector<QVariant> > AbstractQueryUtils::retrieveMasterKeys(qlonglong userId)
{
    QMap<QString, QVariant> bindValues;
    QVector<QVector<QVariant> > results;
    bindValues[QLatin1String(":userid")] = userId;

    executeDirectSql(QLatin1String("SELECT * from MasterKeys where userid=:userid"),
                     bindValues,
                     results);
    return results;
}

bool AbstractQueryUtils::removeUser(int userId, QString userName)
{
    QMap<QString, QVariant> bindValues;
    QVariant id;
    bindValues[QLatin1String(":userid")] = userId;

    AbstractQueryUtils::QueryState rez = executeSql(QLatin1String("DELETE from Users where id=:userid"),
                                                    bindValues,
                                                    id);

    revokeAllPrivileges(userName);
    deleteMySqlUser(userName);
    deleteMySqlUser(userName,QLatin1String("localhost"));

    if(rez == AbstractQueryUtils::NoErrors)
        return true;
    else
        return false;
}

bool AbstractQueryUtils::removeMasterKey(QString keyName)
{
    QMap<QString, QVariant> bindValues;
    QVariant id;
    bindValues[QLatin1String(":keyName")] = keyName;

    AbstractQueryUtils::QueryState rez = executeSql(QLatin1String("DELETE from MasterKeys where keyName=:keyName"),
                                                 bindValues,
                                                 id);

    if(rez == AbstractQueryUtils::NoErrors)
        return true;
    else
        return false;
}

bool AbstractQueryUtils::removeAllMasterKeys(int userid)
{
    QMap<QString, QVariant> bindValues;
    QVariant id;
    bindValues[QLatin1String(":userid")] = userid;

    AbstractQueryUtils::QueryState rez = executeSql(QLatin1String("DELETE from MasterKeys where userid=:userid"),
                                                 bindValues,
                                                 id);

    if(rez == AbstractQueryUtils::NoErrors)
        return true;
    else
        return false;
}






