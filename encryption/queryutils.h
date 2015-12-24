#ifndef QUERYUTILS_H
#define QUERYUTILS_H
#include <QObject>

#include <QMap>


/**
 * @brief The UserDetails class is a container class for storing information
 *        about the use after it is being added to database
 */
class UserDetails
{
public:

    UserDetails()
    {
        id = -1;
    }

    UserDetails(qlonglong id, QString aesFilling)
    {
        this->id = id;
        this->aesFilling = aesFilling;
    }

    UserDetails(const UserDetails& copy)
    {
        this->id = copy.id;
        this->aesFilling = copy.aesFilling;
    }

    qlonglong id;
    QString aesFilling;
};

class QueryUtils : public QObject
{
    Q_OBJECT
public:
    enum UserType{
        ADMIN  = 0,
        USER = 1
    };

    explicit QueryUtils(QObject *parent = 0);

    static QString generateRandomString(int length);

    static QString encpryptMasterKey(QString password, QString filling, QString masterKey);

    static QString decryptMasterKey(QString password, QString filling, QString masterHash);

    static QVector<QVector<QVariant> > retrieveMasterKeys(qlonglong userId);

signals:

public slots:
};

#endif // QUERYUTILS_H
