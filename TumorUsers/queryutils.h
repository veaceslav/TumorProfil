#ifndef QUERYUTILS_H
#define QUERYUTILS_H
#include <QObject>


class QueryUtils : public QObject
{
    Q_OBJECT
public:
    enum UserType{
        ADMIN  = 0,
        NORMAL = 1
    };

    explicit QueryUtils(QObject *parent = 0);

    static bool addUser(QString name, UserType userType, QString password, QString masterKey);

    static QString generateRandomString(int length);



signals:

public slots:
};

#endif // QUERYUTILS_H
