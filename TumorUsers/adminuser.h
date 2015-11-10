#ifndef ADMINUSER_H
#define ADMINUSER_H

#include <QObject>
#include <QPointer>
#include <QVector>

class AdminUser  : public QObject
{
public:
    static AdminUser* instance();
    bool logIn();

protected:
    static QPointer<AdminUser> internalPtr;
    AdminUser();


private:
    void setData(QString password, QVector<QVector<QVariant> >& queryResult);
    class Private;
    Private* d;
};

#endif // ADMINUSER_H
