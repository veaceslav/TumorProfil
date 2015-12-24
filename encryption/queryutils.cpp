#include "queryutils.h"

#include <QTime>
#include <QMap>
#include <QVariant>
#include <QDebug>
#include <QCryptographicHash>

#include "TumorUsers/aesutils.h"
#include "databaseaccess.h"

#define SALT_SIZE  10
#define MASTERKEY_SIZE 40
#define AESKEY_LENGTH  192

QueryUtils::QueryUtils(QObject *parent) : QObject(parent)
{

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



QVector<QVector<QVariant> > QueryUtils::retrieveMasterKeys(qlonglong userId)
{
    QMap<QString, QVariant> bindValues;
    QVector<QVector<QVariant> > results;
    bindValues[QLatin1String(":userid")] = userId;

//    DatabaseAccess::instance()->executeDirectSql(QLatin1String("SELECT * from MasterKeys where userid=:userid"),
//                                                 bindValues,
//                                                 results);
    return results;
}




