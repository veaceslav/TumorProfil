#include "userinformation.h"

#include <QTimer>
#include <QMap>
#include <QDebug>
#include <QMutex>

#include "encryption/authenticationwindow.h"
#include "encryption/queryutils.h"
#include "settings/encryptionsettings.h"
#include "ui/logininfowidget.h"
#include "settings/databasesettings.h"
#include "settings/mainsettings.h"

QPointer<UserInformation> UserInformation::internalPtr = QPointer<UserInformation>();


class UserInformation::Private
{
public:
    Private()
    {
        isLoggedIn = false;
        encryptionEnabled = false;
    }

    bool isLoggedIn;
    bool encryptionEnabled;
    QMutex mutex;
    QString userName;
    QString password;
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


    d->isLoggedIn = true;
    d->userName = data.username;
    d->password = data.password;

    if(d->encryptionEnabled)
    {
        loadKeys();
    }

    LoginInfoWidget::instance()->logInUpdate(data.username);
    emit signalLoginStateChanged();

    return true;
}

bool UserInformation::logOut()
{
    if(!d->isLoggedIn)
    {
        return false;
    }
    d->userName.clear();
    d->password.clear();
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

void UserInformation::setEncryptionEnabled(bool value)
{
    QMutexLocker(&d->mutex);
    d->encryptionEnabled = value;
    if(d->isLoggedIn)
    {
        if(value)
            loadKeys();
        else
            d->decryptionKey.clear();
    }
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

bool UserInformation::loadKeys()
{
    UserDetails details = QueryUtils::retrieveUser(d->userName, d->password);
    if(details.id == -1)
        return false;
    d->decryptionKey = details.decryptionKeys;

    return true;
}


QString UserInformation::retrieveKey(QString keyName)
{
    return d->decryptionKey.value(keyName);
}

void UserInformation::setUsername(QString username)
{
    d->userName = username;
}

void UserInformation::setPassword(QString password)
{
    d->password = password;
}

QString UserInformation::username()
{
    return d->userName;
}

QString UserInformation::password()
{
    return d->password;
}


UserInformation::UserInformation()
    :d(new Private())
{
    QMutexLocker(&d->mutex);
    d->encryptionEnabled = EncryptionSettings::isEncryptionEnabled();
}

bool UserInformation::checkDbConnection(QString username, QString password)
{
    DatabaseParameters params;
    params.readFromConfig();

    params.userName = username;
    params.password = password;

    bool result = DatabaseSettings::checkDatabaseConnection(params);

    if(!result)
    {
        /** Db Options only **/
        MainSettings* settingsDialog = new MainSettings(true);
        settingsDialog->exec();
    } else {
        return true;
    }

    return false;
}
