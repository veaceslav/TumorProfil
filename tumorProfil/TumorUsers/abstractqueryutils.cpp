#include "abstractqueryutils.h"

#include <QTime>
#include <QMap>
#include <QVariant>
#include <QDebug>
#include <QCryptographicHash>

#include "aesutils.h"
#include "time.h"
#include "tumoruserconstants.h"

#define SALT_SIZE  10



AbstractQueryUtils::AbstractQueryUtils(QObject *parent) : QObject(parent)
{

}


UserDetails AbstractQueryUtils::addUser(const QString& name, AbstractQueryUtils::UserType userType, const QString& password)
{
   QMap<QString, QVariant> bindValues;


   QString salt = generateRandomString(SALT_SIZE);

   // Compute password
   QString saltedPass(password + salt);
   QByteArray passHash = QCryptographicHash::hash(saltedPass.toLatin1(), QCryptographicHash::Sha256);



   bindValues[QLatin1String(":name")] = name;

   if(userType == AbstractQueryUtils::ADMIN)
       bindValues[QLatin1String(":usergroup")] = "ADMIN";
   else
       bindValues[QLatin1String(":usergroup")] = "USER";

   bindValues[QLatin1String(":passwordSalt")] = salt;
   bindValues[QLatin1String(":passwordHash")] = QVariant(passHash);

   QVariant id;
   executeSql(QLatin1String("INSERT into Users(name, usergroup, passwordSalt,"
                            " passwordHash)"
                            "VALUES(:name, :usergroup, :passwordSalt, :passwordHash)"),
              bindValues,
              id);

    // According to MySql documentation, we add a user with wildcard "%", but we also need to add the same user
    // with localhost
   if(name != QLatin1String(ADMIN_NAME))
   {
       addMySqlUser(name,password);
       addMySqlUser(name,password, QLatin1String("localhost"));
   }

   return UserDetails(id.toLongLong(), salt);
}

UserDetails AbstractQueryUtils::editUser(const QString& name, AbstractQueryUtils::UserType userType, const QString& password, qlonglong userId)
{
    QMap<QString, QVariant> bindValues;
    QVector<QVector<QVariant> > results;

    bindValues[QLatin1String(":id")] = userId;
    executeDirectSql(QLatin1String("SELECT name from Users where id=:id"),
                     bindValues,
                     results);

    if (results.isEmpty())
    {
        qWarning() << "Did not identify user" << name << "when editing user details";
        return UserDetails();
    }
    QString oldName = results.first().first().toString();
    bindValues.clear();
    results.clear();

    if (name != oldName)
    {
        if (!executeDirectSql(QString("RENAME USER %1 TO %2").arg(oldName).arg(name), bindValues, results))
        {
            qWarning() <<"Failed to change username" << oldName << "to" << name;
        }
    }

    QString salt = generateRandomString(SALT_SIZE);

    // Compute password
    QString saltedPass(password + salt);
    QByteArray passHash = QCryptographicHash::hash(saltedPass.toLatin1(), QCryptographicHash::Sha256);

    bindValues[QLatin1String(":name")] = name;

    if(userType == AbstractQueryUtils::ADMIN)
        bindValues[QLatin1String(":usergroup")] = "ADMIN";
    else
        bindValues[QLatin1String(":usergroup")] = "USER";

    bindValues[QLatin1String(":passwordSalt")] = salt;
    bindValues[QLatin1String(":passwordHash")] = QVariant(passHash);
    bindValues[QLatin1String(":userid")] = userId;

    QVariant id;
    executeSql(QLatin1String("UPDATE Users"
                             " SET name= :name, usergroup= :usergroup, passwordSalt= :passwordSalt, "
                             " passwordHash= :passwordHash"
                             " WHERE id = :userid"),
               bindValues,
               id);


    setMySqlPassword(name, password);

    return UserDetails(userId, salt);
}

bool AbstractQueryUtils::updateUserMasterKeys(int userId, const QString& userPassword, const QString& salt,
                                      QMap<QString,QString> userKeys)
{
    AbstractQueryUtils::removeAllMasterKeys(userId);

    QMap<QString, QString>::iterator it;
    for(it=userKeys.begin(); it != userKeys.end(); ++it)
    {
        AbstractQueryUtils::addMasterKey(it.key(), userId, userPassword, salt,
                                 it.value());
    }

    return true;
}

bool AbstractQueryUtils::addMySqlUser(const QString& user, const QString& password, const QString& host)
{
    QMap<QString, QVariant> bindValues;
    QVector<QVector<QVariant> > results;

    QString query = QString("CREATE USER '%1'@'%2' IDENTIFIED BY '%3'").arg(user, host, password);
    executeDirectSql(query,
                     bindValues,
                     results);
    return true;
}

bool AbstractQueryUtils::deleteMySqlUser(const QString& user, const QString& host)
{
    QMap<QString, QVariant> bindValues;
    QVector<QVector<QVariant> > results;

    QString query = QString("DROP USER IF EXISTS '%1'@'%2'").arg(user, host);
    executeDirectSql(query,
                     bindValues,
                     results);

    return true;
}

bool AbstractQueryUtils::setMySqlPassword(const QString& user, const QString& password, const QString& host)
{
    QMap<QString, QVariant> bindValues;
    QVector<QVector<QVariant> > results;

    QString query = QString("SET PASSWORD FOR '%1'@'%2' = PASSWORD('%3')").arg(user, host, password);
    executeDirectSql(query,
                     bindValues,
                     results);
    return true;
}

bool AbstractQueryUtils::grantMySqlPermissions(const QString& user, const QString& databaseName, const QString& userHost, const QString& tableName, const QString& type)
{
    QMap<QString, QVariant> bindValues;
    QVector<QVector<QVariant> > results;

    QString query = QString("GRANT %1 ON %2.%3 TO '%4'@'%5'").arg(type,
                                                                  databaseName,
                                                                  tableName,
                                                                  user,
                                                                  userHost);
    executeDirectSql(query,
                     bindValues,
                     results);
    qDebug() << query;
    return true;
}

QVector<QString> AbstractQueryUtils::getTumorProfilTables(const QString& tumorProfilDbName)
{
    QMap<QString, QVariant> bindValues;
    QVector<QVector<QVariant> > results;
    QVector<QString> result;

    QString query = QString("SHOW TABLES FROM %1").arg(tumorProfilDbName);
    executeDirectSql(query,
                     bindValues,
                     results);

    for(QVector<QVariant> val : results)
    {
        result.append(val.first().toString());
    }

    return result;
}

bool AbstractQueryUtils::revokeAllPrivileges(const QString& user)
{
    qDebug() << "Revoking privileges for " << user;
    QMap<QString, QVariant> bindValues;
    QVector<QVector<QVariant> > results;

    QString query = QString("REVOKE ALL PRIVILEGES, GRANT OPTION FROM '%1'@'localhost',"
                            "'%2'@'%'").arg(user, user);
    executeDirectSql(query,
                     bindValues,
                     results);

    return true;
}

void AbstractQueryUtils::updatePermissionMap(QMap<QString, int> &permissionMap, const QString& tableName, const QString& grantOption)
{

    int permission;
    if (grantOption == "SELECT")
    {
        permission = (int)AbstractQueryUtils::PERMISSION_READ;
    }
    else if (grantOption == "ALL")
    {
        permission = (int)AbstractQueryUtils::PERMISSION_READWRITE;
    }

    if (tableName == "*")
    {
        for(const QString& key : permissionMap.keys()) // we need a temporary copy of keys because we modify the map here
        {
            if(permissionMap[key] < permission) // We set maximum of available permissions
            {
                permissionMap[key] = permission;
            }
        }
    }
    else
    {
        permissionMap[tableName] = permission;
    }
}

QMap<QString, int> AbstractQueryUtils::getPermissions(const QString& tumorProfilDatabaseName, const QString& userName)
{
    QMap<QString, QVariant> bindValues;
    QVector<QVector<QVariant> > results;
    QMap<QString, int> permissionMap;

    // Initializing all permissions to none
    for(const QString& tableName : getTumorProfilTables(tumorProfilDatabaseName))
    {
        permissionMap.insert(tableName, AbstractQueryUtils::PERMISSION_NONE);
    }

    bindValues[":userName"] = userName;
    QString query("SELECT tableName, type FROM UserAccess JOIN Users on Users.id=UserAccess.userid WHERE Users.name=:userName");
    executeDirectSql(query, bindValues, results);

    for(QVector<QVariant> rezVector : results)
    {
        updatePermissionMap(permissionMap, rezVector.first().toString(), rezVector.last().toString());
    }

    // Does not work because mysql_stmt_result_metadata returns null for the statement, so isSelect() is false and no values are returned
    /*
    QString query = QString("SHOW GRANTS FOR %1").arg(user);
    qDebug() << "Grants query:" << query;
    executeDirectSql(query,
                     bindValues,
                     results);

    qDebug() << "Grants:" << query << results;

    for(QVector<QVariant> rezVector : results)
    {
        for(QVariant data : rezVector)
        {
            qDebug() << data.toString() ;
            QString stringData = data.toString().remove("`");
            QStringList splitData = stringData.split(" ");

            QString permission = splitData.at(1); // ALL or SELECT

            QStringList tokens;
            if(permission == QLatin1String("ALL"))
                tokens =  splitData.at(4).split(".");
            else
                tokens = splitData.at(3).split(".");

            if(tokens.size() == 2) // other permissions
            {
                if(tokens.first() == databaseName || tokens.first() == QLatin1String("*")){
                    updatePermissionMap(permissionMap, permission, tokens.at(1));
                }
            }
        }
    }
    */
    return permissionMap;
}



QString AbstractQueryUtils::generateRandomString(int length)
{
    qsrand(time(NULL));
    QString base("0123456789ABCDEF");
    QString str;
    for(int i=0;i<length;i++){
        str += base.at(qrand() % base.length());
    }

    return str;

}


qlonglong AbstractQueryUtils::addMasterKey(const QString& name, qlonglong userid, const QString& password, const QString& salt, const QString& givenMasterKey)
{
    QString masterKey(givenMasterKey);
    if(masterKey.isEmpty())
        masterKey = generateRandomString(AESKEY_LENGTH);

    QString encodedKey = AesUtils::encryptMasterKey(password, salt, masterKey);

    QString decoded = AesUtils::decryptMasterKey(password, salt, encodedKey);

    if(decoded.compare(masterKey) != 0)
        qDebug() << "Wrong encryption. Expected: " << masterKey << " Got: " << decoded;

    QMap<QString, QVariant> bindValues;
    bindValues[QLatin1String(":keyName")] = name;
    bindValues[QLatin1String(":userid")] = userid;
    bindValues[QLatin1String(":encryptedKey")] = encodedKey;

    QVariant id;
    executeSql(QLatin1String("INSERT into MasterKeys(keyName, userid, encryptedKey)"
                             "VALUES(:keyName, :userid, :encryptedKey)"),
               bindValues,
               id);

    return id.toLongLong();
}

QVector<QVector<QVariant> > AbstractQueryUtils::retrieveMasterKeys(qlonglong userId)
{
    QMap<QString, QVariant> bindValues;
    QVector<QVector<QVariant> > results;
    bindValues[QLatin1String(":userid")] = userId;

    executeDirectSql(QLatin1String("SELECT * from MasterKeys where userid=:userid"),
                     bindValues,
                     results);
    return results;
}

bool AbstractQueryUtils::removeUser(int userId, const QString& userName)
{
    QMap<QString, QVariant> bindValues;
    QVariant id;
    bindValues[QLatin1String(":userid")] = userId;

    AbstractQueryUtils::QueryState rez = executeSql(QLatin1String("DELETE from Users where id=:userid"),
                                                    bindValues,
                                                    id);

    revokeAllPrivileges(userName);
    deleteMySqlUser(userName);
    deleteMySqlUser(userName,QLatin1String("localhost"));

    if(rez == AbstractQueryUtils::NoErrors)
        return true;
    else
        return false;
}

bool AbstractQueryUtils::removeMasterKey(const QString& keyName)
{
    QMap<QString, QVariant> bindValues;
    QVariant id;
    bindValues[QLatin1String(":keyName")] = keyName;

    AbstractQueryUtils::QueryState rez = executeSql(QLatin1String("DELETE from MasterKeys where keyName=:keyName"),
                                                 bindValues,
                                                 id);

    if(rez == AbstractQueryUtils::NoErrors)
        return true;
    else
        return false;
}

bool AbstractQueryUtils::removeAllMasterKeys(int userid)
{
    QMap<QString, QVariant> bindValues;
    QVariant id;
    bindValues[QLatin1String(":userid")] = userid;

    AbstractQueryUtils::QueryState rez = executeSql(QLatin1String("DELETE from MasterKeys where userid=:userid"),
                                                 bindValues,
                                                 id);

    if(rez == AbstractQueryUtils::NoErrors)
        return true;
    else
        return false;
}

bool AbstractQueryUtils::clearRecordedPermissions(qlonglong userid)
{
    QMap<QString, QVariant> bindValues;
    QVariant id;
    bindValues[QLatin1String(":userid")] = userid;

    AbstractQueryUtils::QueryState rez = executeSql(QLatin1String("DELETE from UserAccess where userid=:userid"),
                                                 bindValues,
                                                 id);

    if(rez == AbstractQueryUtils::NoErrors)
        return true;
    else
        return false;
}

bool AbstractQueryUtils::recordPermission(qlonglong userId, const QString &tableName, const QString &type)
{
    QMap<QString, QVariant> bindValues;
    bindValues[":userid"] = userId;
    bindValues[":tableName"] = tableName;
    bindValues[":type"] = type;
    QVariant id;
    AbstractQueryUtils::QueryState rez = executeSql("INSERT INTO UserAccess(userid, tableName, type) VALUES (:userid, :tableName, :type)", bindValues, id);

    if(rez == AbstractQueryUtils::NoErrors)
        return true;
    else
        return false;
}




