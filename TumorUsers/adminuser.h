#ifndef ADMINUSER_H
#define ADMINUSER_H

#include <QObject>
#include <QPointer>

class AdminUser  : public QObject
{
public:
    static AdminUser* instance();
    bool logIn();

protected:
    static QPointer<AdminUser> internalPtr;
    AdminUser();


private:
    class Private;
    Private* d;
};

#endif // ADMINUSER_H
