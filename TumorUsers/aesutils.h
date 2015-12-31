#ifndef AESUTILS_H
#define AESUTILS_H

#define AESKEY_LENGTH  256

#include <QString>

class AesUtils
{
public:
    AesUtils();

    static QString encryptMasterKey(QString password, QString filling, QString masterKey);

    static QString decryptMasterKey(QString password, QString filling, QString masterHash);

    static QString encrypt(QString message, QString aesKey);

    static QString decrypt(QString message, QString aesKey);
};

#endif // AESUTILS_H
