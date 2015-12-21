#ifndef AESUTILS_H
#define AESUTILS_H


#include <QString>

class AesUtils
{
public:
    AesUtils();

    static QString encrypt(QString message, QString aesKey);

    static QString decrypt(QString message, QString aesKey);
};

#endif // AESUTILS_H
