#ifndef ADMINUSER_H
#define ADMINUSER_H

#include <QObject>
#include <QPointer>
#include <QVector>


class MasterKey
{
public:


    enum Fied
    {
        NAME_FIELD = 1,
        VALUE_FIELD = 2
    };

    MasterKey()
    {

    }

    MasterKey(QString name, QString value)
    {
        this->name = name;
        this->value = value;
    }
    MasterKey(const MasterKey& copy)
    {
        this->name = copy.name;
        this->value = copy.value;
    }

    QString name;
    QString value;
};

class AdminUser  : public QObject
{

    enum FIELD_NAMES{
        ID_FIELD = 0,
        NAME_FIELD = 1,
        USERGROUP  = 2,
        PASSWORD_SALT = 3,
        PASS_HASH     = 4,
        AES_FILLING   = 5,
        ENCRYPTED_KEY = 6
    };
public:
    static AdminUser* instance();
    bool logIn();
    QString masterKey();

protected:
    static QPointer<AdminUser> internalPtr;
    AdminUser();


private:
    void setData(QString password, QVector<QVector<QVariant> >& queryResult);
    class Private;
    Private* d;
};

#endif // ADMINUSER_H
