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
        :id(-1)
    {
        id = -1;
    }

    UserDetails(qlonglong id, const QString& salt)
        : id(id), userSalt(salt)
    {
    }

    qlonglong id;
    QString userName;
    QString userSalt;
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

    enum UserPermissions{
        PERMISSION_NONE = 0,
        PERMISSION_READ = 1,
        PERMISSION_READWRITE = 2
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



    UserDetails addUser(const QString& name, UserType userType, const QString& password);


    QString generateRandomString(int length);

    qlonglong addMasterKey(const QString& name, qlonglong userid, const QString& password, const QString& salt, const QString& masterKey = QString());

    QVector<QVector<QVariant> > retrieveMasterKeys(qlonglong userId);

    bool removeUser(int userId, const QString& userName);

    bool removeMasterKey(const QString& keyName);

    bool removeAllMasterKeys(int userid);

    UserDetails editUser(const QString& name, UserType userType, const QString& password, qlonglong userId);

    bool updateUserMasterKeys(int userId, const QString& userPassword, const QString& salt,
                              QMap<QString, QString> userKeys);

    bool addMySqlUser(const QString& user, const QString& password, const QString& host = QString("%"));

    bool deleteMySqlUser(const QString& user, const QString& host = QString("%"));

    bool setMySqlPassword(const QString& user, const QString& password, const QString& host = QString("%"));

    bool grantMySqlPermissions(const QString& user,  const QString& databaseName, const QString& userHost = QString("%"), const QString& tableName = QString("*"),
                               const QString& type=QString("ALL"));

    bool clearRecordedPermissions(qlonglong userid);
    bool recordPermission(qlonglong userid, const QString& tableName, const QString& type);

    QVector<QString> getTumorProfilTables(const QString& tumorProfilDbName);

    bool revokeAllPrivileges(const QString& user);

    QMap<QString, int> getPermissions(const QString& databaseName, const QString& userName);

    virtual QueryState executeSql(const QString& queryString, QMap<QString,
                                  QVariant> bindValues, QVariant& lastId, const QString& databaseID = QString()) = 0;

    virtual QueryState executeDirectSql(const QString& queryString, QMap<QString,
                                        QVariant> bindValues, QVector<QVector<QVariant> >& results,
                                        const QString& databaseID = QString()) = 0;


protected:
    explicit AbstractQueryUtils(QObject *parent = 0);

private:
    /**
     * @brief updatePermissionMap - utility for getting user permissions
     * @param permissionMap
     * @param grantOption
     * @param tableName
     */
    void updatePermissionMap(QMap<QString, int>& permissionMap, const QString& tableName, const QString& grantOption);


};

#endif // QUERYUTILS_H
