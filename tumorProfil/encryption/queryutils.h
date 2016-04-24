#ifndef QUERYUTILS_H
#define QUERYUTILS_H
#include <QObject>

#include <QMap>
#include "databaseparameters.h"


class QSqlDatabase;


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


    UserDetails(const UserDetails& copy)
    {
        this->id = copy.id;
        this->userName = copy.userName;
        this->decryptionKeys = QMap<QString, QString>(copy.decryptionKeys);
    }

    qlonglong id;
    QString userName;
    QMap<QString, QString> decryptionKeys;
};

class TumorQueryUtils : public QObject
{
    Q_OBJECT
public:
    enum UserType{
        ADMIN  = 0,
        USER = 1
    };

    enum DatabaseName{
        TUMORPROFIL = 0,
        TUMORUSER = 1
    };

    explicit TumorQueryUtils(QObject *parent = 0);


    static QString encpryptMasterKey(QString password, QString filling, QString masterKey);

    static QString decryptMasterKey(QString password, QString filling, QString masterHash);

    static QVector<QVector<QVariant> > retrieveMasterKeys(qlonglong userId, QString databaseID);

    static QVector<QVector<QVariant> > retrieveUserEntry(const QString& userName, QString databaseID);

    static UserDetails retrieveUser(QString name, QString password);

    static bool openConnection(DatabaseParameters params, QString databaseID, DatabaseName databaseName);

    static bool closeConnection(QString databaseID);

    static bool executeDirectSql(QString queryString, QMap<QString,
                                 QVariant> bindValues, QVector<QVector<QVariant> >& results,
                                 QString databaseID);

    static bool verifyPassword(const QString& password , const QVector<QVector<QVariant> >& result);

    static bool removeAllMasterKeys(int userid, QString databaseID);

    static bool updateUserMasterKeys(int userId, QString userPassword, QString userAesFilling,
                                          QMap<QString,QString> userKeys, QString databaseID);
    static qlonglong addMasterKey(QString name, qlonglong userid, QString password,
                                  QString aesFilling,  QString databaseID,QString masterKey = QString());


signals:

public slots:
};

#endif // QUERYUTILS_H
