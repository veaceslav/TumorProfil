#ifndef QUERYUTILS_H
#define QUERYUTILS_H
#include <QObject>

#include <QMap>
#include "databaseparameters.h"


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

class QueryUtils : public QObject
{
    Q_OBJECT
public:
    enum UserType{
        ADMIN  = 0,
        USER = 1
    };

    explicit QueryUtils(QObject *parent = 0);


    static QString encpryptMasterKey(QString password, QString filling, QString masterKey);

    static QString decryptMasterKey(QString password, QString filling, QString masterHash);

    static QVector<QVector<QVariant> > retrieveMasterKeys(qlonglong userId);

    static QVector<QVector<QVariant> > retrieveUserEntry(const QString& userName);

    static UserDetails retrieveUser(QString name, QString password);

    static bool openConnection(DatabaseParameters params);

    static bool executeDirectSql(QString queryString, QMap<QString, QVariant> bindValues, QVector<QVector<QVariant> >& results);

    static bool verifyPassword(const QString& password , const QVector<QVector<QVariant> >& result);

signals:

public slots:
};

#endif // QUERYUTILS_H
