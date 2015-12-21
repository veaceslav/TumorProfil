#include "queryutils.h"

#include <QTime>
#include <QMap>
#include <QVariant>
#include <QDebug>
#include <QCryptographicHash>

#include "aesutils.h"
#include "databaseaccess.h"

#define SALT_SIZE  10
#define MASTERKEY_SIZE 40
#define AESKEY_LENGTH  192

QueryUtils::QueryUtils(QObject *parent) : QObject(parent)
{

}

UserDetails QueryUtils::addUser(QString name, QueryUtils::UserType userType, QString password)
{
   QMap<QString, QVariant> bindValues;


   QString salt = generateRandomString(SALT_SIZE);

   // Compute password
   QString saltedPass(password + salt);
   QByteArray passHash = QCryptographicHash::hash(saltedPass.toLatin1(), QCryptographicHash::Sha256);

   // add aes private key filling
   QString aesFilling = generateRandomString(AESKEY_LENGTH);


   bindValues[QLatin1String(":name")] = name;

   if(userType == QueryUtils::ADMIN)
       bindValues[QLatin1String(":usergroup")] = "ADMIN";
   else
       bindValues[QLatin1String(":usergroup")] = "USER";

   bindValues[QLatin1String(":passwordSalt")] = salt;
   bindValues[QLatin1String(":passwordHash")] = QVariant(passHash);
   bindValues[QLatin1String(":aesPrivateKeyFilling")] = aesFilling;

   QVariant id;
   DatabaseAccess::instance()->executeSql(QLatin1String("INSERT into Users(name, usergroup, passwordSalt,"
                                                              " passwordHash, aesPrivateKeyFilling)"
                                                              "VALUES(:name, :usergroup, :passwordSalt, :passwordHash,"
                                                              ":aesPrivateKeyFilling)"),
                                                bindValues,
                                                id);


   qDebug() << "Id of inserted item: " << id;
   return UserDetails(id.toLongLong(),aesFilling);
}

UserDetails QueryUtils::editUser(QString name, QueryUtils::UserType userType, QString password, qlonglong userId)
{
    QMap<QString, QVariant> bindValues;


    QString salt = generateRandomString(SALT_SIZE);

    // Compute password
    QString saltedPass(password + salt);
    QByteArray passHash = QCryptographicHash::hash(saltedPass.toLatin1(), QCryptographicHash::Sha256);

    // add aes private key filling
    QString aesFilling = generateRandomString(AESKEY_LENGTH);


    bindValues[QLatin1String(":name")] = name;

    if(userType == QueryUtils::ADMIN)
        bindValues[QLatin1String(":usergroup")] = "ADMIN";
    else
        bindValues[QLatin1String(":usergroup")] = "USER";

    bindValues[QLatin1String(":passwordSalt")] = salt;
    bindValues[QLatin1String(":passwordHash")] = QVariant(passHash);
    bindValues[QLatin1String(":aesPrivateKeyFilling")] = aesFilling;
    bindValues[QLatin1String(":userid")] = userId;

    QVariant id;
    DatabaseAccess::instance()->executeSql(QLatin1String("UPDATE Users"
                                                         " SET name= :name, usergroup= :usergroup, passwordSalt= :passwordSalt, "
                                                         " passwordHash= :passwordHash, aesPrivateKeyFilling= :aesPrivateKeyFilling"
                                                         " WHERE id = :userid"),
                                                 bindValues,
                                                 id);


    qDebug() << "Id of inserted item: " << userId;
    return UserDetails(userId,aesFilling);
}

QString QueryUtils::generateRandomString(int length)
{
    qsrand(time(NULL));
    QString base("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
    QString str;
    for(int i=0;i<length;i++){
        str += base.at(qrand() % base.length());
    }

    return str;
}

QString QueryUtils::encpryptMasterKey(QString password, QString filling, QString masterKey)
{
    QString aesKey = password + filling;
    aesKey.truncate(AESKEY_LENGTH);
    return AesUtils::encrypt(masterKey, aesKey);
}

QString QueryUtils::decryptMasterKey(QString password, QString filling, QString masterHash)
{
    QString decryption = password + filling;
    decryption.truncate(AESKEY_LENGTH);
    return AesUtils::decrypt(masterHash, decryption);
}

qlonglong QueryUtils::addMasterKey(QString name, qlonglong userid, QString password, QString aesFilling, QString masterKey)
{

    if(masterKey.isEmpty())
        masterKey = generateRandomString(MASTERKEY_SIZE);

    QString aesKey = password + aesFilling;
    aesKey.truncate(64);
    QString encodedKey = AesUtils::encrypt(masterKey, aesKey);

    QMap<QString, QVariant> bindValues;
    bindValues[QLatin1String(":keyName")] = name;
    bindValues[QLatin1String(":userid")] = userid;
    bindValues[QLatin1String(":encryptedKey")] = encodedKey;

    QVariant id;
    DatabaseAccess::instance()->executeSql(QLatin1String("INSERT into MasterKeys(keyName, userid, encryptedKey)"
                                                               "VALUES(:keyName, :userid, :encryptedKey)"),
                                                 bindValues,
                                                 id);

    return id.toLongLong();
}

QVector<QVector<QVariant> > QueryUtils::retrieveMasterKeys(qlonglong userId)
{
    QMap<QString, QVariant> bindValues;
    QVector<QVector<QVariant> > results;
    bindValues[QLatin1String(":userid")] = userId;

    DatabaseAccess::instance()->executeDirectSql(QLatin1String("SELECT * from MasterKeys where userid=:userid"),
                                                 bindValues,
                                                 results);
    return results;
}

bool QueryUtils::removeUser(int userId)
{
    QMap<QString, QVariant> bindValues;
    QVariant id;
    bindValues[QLatin1String(":userid")] = userId;

    DatabaseAccess::QueryStateEnum rez = DatabaseAccess::instance()->executeSql(QLatin1String("DELETE from Users where id=:userid"),
                                                 bindValues,
                                                 id);

    if(rez == DatabaseAccess::NoErrors)
        return true;
    else
        return false;
}

bool QueryUtils::removeMasterKey(QString keyName)
{
    QMap<QString, QVariant> bindValues;
    QVariant id;
    bindValues[QLatin1String(":keyName")] = keyName;

    DatabaseAccess::QueryStateEnum rez = DatabaseAccess::instance()->executeSql(QLatin1String("DELETE from MasterKeys where keyName=:keyName"),
                                                 bindValues,
                                                 id);

    if(rez == DatabaseAccess::NoErrors)
        return true;
    else
        return false;
}

bool QueryUtils::removeAllMasterKeys(int userid)
{
    QMap<QString, QVariant> bindValues;
    QVariant id;
    bindValues[QLatin1String(":userid")] = userid;

    DatabaseAccess::QueryStateEnum rez = DatabaseAccess::instance()->executeSql(QLatin1String("DELETE from MasterKeys where userid=:userid"),
                                                 bindValues,
                                                 id);

    if(rez == DatabaseAccess::NoErrors)
        return true;
    else
        return false;
}



