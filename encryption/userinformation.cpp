#include "userinformation.h"

#include <QTimer>
#include <QMap>
#include <QDebug>

#include "authenticationwindow.h"
#include "queryutils.h"
#include "settings/encryptionsettings.h"
#include "ui/logininfowidget.h"

QPointer<UserInformation> UserInformation::internalPtr = QPointer<UserInformation>();


class UserInformation::Private
{
public:
    Private()
    {
        isLoggedIn = false;
    }

    bool isLoggedIn;
    bool encryptionEnabled;
    QString userName;
    QMap<QString, QString> decryptionKey;

    QTimer timer; // set up logout timeout.
};

UserInformation *UserInformation::instance()
{
    if(UserInformation::internalPtr.isNull())
        UserInformation::internalPtr = new UserInformation();

    return UserInformation::internalPtr;
}

bool UserInformation::logIn()
{
    UserData data = AuthenticationWindow::logIn();
    UserDetails details = QueryUtils::retrieveUser(data.username, data.password);
    if(details.id == -1)
        return false;

    d->isLoggedIn = true;
    d->decryptionKey = details.decryptionKeys;
    d->userName = details.userName;
    LoginInfoWidget::instance()->logInUpdate(details.userName);
    emit signalLoginStateChanged();

    return true;
}

bool UserInformation::logOut()
{
    if(!d->isLoggedIn)
    {
        return false;
    }
    d->userName = QString();
    d->decryptionKey.clear();
    d->isLoggedIn = false;
    LoginInfoWidget::instance()->logOutUpdate();
    emit signalLoginStateChanged();

    return true;
}

bool UserInformation::isEncryptionEnabled()
{
    return d->encryptionEnabled;
}

bool UserInformation::isLoggedIn()
{
    //qDebug() << "Is logged in" << d->isLoggedIn;
    return d->isLoggedIn;
}

UserInformation::LoginState UserInformation::toggleLogIn()
{
    if(!d->isLoggedIn)
    {
        if(logIn())
            return UserInformation::LOGGEDIN;
        else
            return UserInformation::NOT_LOGGEDIN;
    }
    else
    {
        // No error here
        logOut();
        return UserInformation::NOT_LOGGEDIN;
    }
}

bool UserInformation::hasKey(QString keyName)
{
    return !d->decryptionKey.value(keyName).isEmpty();
}


QString UserInformation::retrieveKey(QString keyName)
{
    return d->decryptionKey.value(keyName);
}


UserInformation::UserInformation()
    :d(new Private())
{
    d->encryptionEnabled = EncryptionSettings::isEncryptionEnabled();
}
