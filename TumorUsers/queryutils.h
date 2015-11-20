#ifndef QUERYUTILS_H
#define QUERYUTILS_H
#include <QObject>

#include <QMap>


class QueryUtils : public QObject
{
    Q_OBJECT
public:
    enum UserType{
        ADMIN  = 0,
        USER = 1
    };

    explicit QueryUtils(QObject *parent = 0);

    static qlonglong addUser(QString name, UserType userType, QString password, QString masterKey);

    static QString generateRandomString(int length);

    static QString encpryptMasterKey(QString password, QString filling, QString masterKey);

    static QString decryptMasterKey(QString password, QString filling, QString masterHash);

    static qlonglong addMasterKey(QString name, qlonglong userid, QString password, QString aesFilling);

    static QVector<QVector<QVariant> > retrieveMasterKeys(qlonglong userId);



signals:

public slots:
};

#endif // QUERYUTILS_H
