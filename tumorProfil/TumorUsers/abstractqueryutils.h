#ifndef ABSTRACTQUERYUTILS_H
#define ABSTRACTQUERYUTILS_H
#include <QObject>
#include <QMap>
#include <QPointer>
#include <QVariant>


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
        this->userName = copy.userName;
        this->decryptionKeys = copy.decryptionKeys;
    }

    qlonglong id;
    QString aesFilling;
    QString userName;
    QMap<QString, QString> decryptionKeys;

};


/**
 * @brief The AbstractQueryUtils class provides implementation for addding, removing editing users and master keys
 *        The class is abstract because it is used in two different applications:
 *          - TumorUsers - for management
 *          - TumorProfil - to change user password
 *        Each program should implement the executeSql() and executeDirectSql() to match their database backends implementation
 */
class AbstractQueryUtils : public QObject
{
    Q_OBJECT
public:
    enum UserType{
        ADMIN  = 0,
        USER = 1
    };

    enum QueryState
    {
        /**
         * No errors occurred while executing the query.
         */
        NoErrors = 0,

        /**
         * An SQLError has occurred while executing the query.
         */
        SQLError = 1,

        /**
         * An connection error has occurred while executing the query.
         */
        ConnectionError = 2
    };



    UserDetails addUser(QString name, UserType userType, QString password);


    QString generateRandomString(int length);

    qlonglong addMasterKey(QString name, qlonglong userid, QString password, QString aesFilling, QString masterKey = QString());

    QVector<QVector<QVariant> > retrieveMasterKeys(qlonglong userId);

    bool removeUser(int userId, QString userName);

    bool removeMasterKey(QString keyName);

    bool removeAllMasterKeys(int userid);

    UserDetails editUser(QString name, UserType userType, QString password, qlonglong userId);

    bool updateUserMasterKeys(int userId, QString userPassword, QString userAesFilling,
                              QMap<QString, QString> userKeys);

    bool addMySqlUser(QString user, QString password, QString host = QString("%"));

    bool deleteMySqlUser(QString user, QString host = QString("%"));

    bool setMySqlPassword(QString user, QString password, QString host = QString("%"));

    bool grantMySqlPermissions(QString user,  QString databaseName, QString userHost = QString("%"));

    QVector<QString> getTumorProfilTables(QString tumorProfilDbName);

    virtual QueryState executeSql(QString queryString, QMap<QString,
                                  QVariant> bindValues, QVariant& lastId, QString databaseID = QString()) = 0;

    virtual QueryState executeDirectSql(QString queryString, QMap<QString,
                                        QVariant> bindValues, QVector<QVector<QVariant> >& results,
                                        QString databaseID = QString()) = 0;


protected:
    explicit AbstractQueryUtils(QObject *parent = 0);


};

#endif // QUERYUTILS_H
