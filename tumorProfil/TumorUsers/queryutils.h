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

    static UserDetails addUser(QString name, UserType userType, QString password);



    static QString generateRandomString(int length);

    static qlonglong addMasterKey(QString name, qlonglong userid, QString password, QString aesFilling, QString masterKey = QString());

    static QVector<QVector<QVariant> > retrieveMasterKeys(qlonglong userId);

    static bool removeUser(int userId);

    static bool removeMasterKey(QString keyName);

    static bool removeAllMasterKeys(int userid);

    static UserDetails editUser(QString name, UserType userType, QString password, qlonglong userId);

    static bool updateUserMasterKeys(int userId, QString userPassword, QString userAesFilling,
                                     QMap<QString, QString> userKeys);

signals:

public slots:
};

#endif // QUERYUTILS_H
