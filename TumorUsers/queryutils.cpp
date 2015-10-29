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
#define AESKEY_LENGTH  64

QueryUtils::QueryUtils(QObject *parent) : QObject(parent)
{

}

bool QueryUtils::addUser(QString name, QueryUtils::UserType userType, QString password, QString masterKey)
{
   QMap<QString, QVariant> bindValues;


   QString salt = generateRandomString(SALT_SIZE);

   // Compute password
   QString saltedPass(password + salt);
   QByteArray passHash = QCryptographicHash::hash(saltedPass.toLatin1(), QCryptographicHash::Sha256);

   qDebug() << "Password hash " << QString(passHash);
   // add aes private key filling
   QString aesFilling = generateRandomString(AESKEY_LENGTH);

   if(userType == QueryUtils::ADMIN && masterKey.isEmpty())
   {
       masterKey = generateRandomString(MASTERKEY_SIZE);
   }
   else
   {
        qDebug() << "Error, not an admin and no masterkey supplied";
        return false;
   }

   QString aesKey = password + aesFilling;
   aesKey.truncate(64);
   QString encodedKey = AesUtils::encrypt(masterKey, aesKey);

   bindValues[QLatin1String(":name")] = name;

   if(userType == QueryUtils::ADMIN)
       bindValues[QLatin1String(":usergroup")] = "ADMIN";
   else
       bindValues[QLatin1String(":usergroup")] = "USER";

   bindValues[QLatin1String(":passwordSalt")] = salt;
   bindValues[QLatin1String(":passwordHash")] = QVariant(QString(passHash));
   bindValues[QLatin1String(":aesPrivateKeyFilling")] = aesFilling;
   bindValues[QLatin1String(":encryptedKey")] = encodedKey;

   QVector<QVector<QVariant> > result;
   DatabaseAccess::instance()->executeDirectSql(QLatin1String("INSERT into Users(name, usergroup, passwordSalt,"
                                                              " passwordHash, aesPrivateKeyFilling,encryptedKey)"
                                                              "VALUES(:name, :usergroup, :passwordSalt, :passwordHash,"
                                                              ":aesPrivateKeyFilling, :encryptedKey)"),
                                                bindValues,
                                                result);


   return true;
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

