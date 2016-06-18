#ifndef AESUTILS_H
#define AESUTILS_H

#define AESKEY_LENGTH  64 // AES key is hex encoded 64x4 = 256

#include <QString>

class AesUtils
{
public:
    AesUtils();

    static QString encryptMasterKey(QString password, QString filling, QString masterKey);

    static QString decryptMasterKey(QString password, QString filling, QString masterHash);

    static QString encrypt(QString message, QString aesKey);

    static QString decrypt(QString message, QString aesKey);

private:
    static QString deriveKey(QString password, QString salt);
};

#endif // AESUTILS_H
