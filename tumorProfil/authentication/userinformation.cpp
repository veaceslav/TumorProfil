#include "userinformation.h"

#include <QTimer>
#include <QMap>
#include <QDebug>
#include <QMutex>
#include <QDialog>
#include <QMessageBox>


#include "databaseaccess.h"
#include "encryption/authenticationwindow.h"
#include "encryption/queryutils.h"
#include "settings/encryptionsettings.h"
#include "ui/logininfowidget.h"
#include "settings/databasesettings.h"
#include "settings/mainsettings.h"
#include "authentication/accessmanagement.h"



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
    QMap<QString, int> permissions;

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

    d->userName = data.username;
    d->password = data.password;

    // DatabaseParameters from config may be changed here and in the loop

    int result = checkDbConnection(d->userName, d->password);

    bool login = true;

    while(login)
    {
        switch(result){
        case UserInformation::ALL_OK: // everything is ok
            login = false;
            break;
        case QDialog::Accepted:
            result = checkDbConnection(d->userName, d->password);
            break;
        case QDialog::Rejected:
            login = false;
            break;
        }
    }

    if(result == UserInformation::ALL_OK)
    {
        if(d->encryptionEnabled)
        {
            loadKeys();
        }

        // Create initial config
        DatabaseParameters params;
        params.readFromConfig();
        params.userName = d->userName;
        params.password = d->password;
        DatabaseAccess::setParameters(params);
        TumorQueryUtils::open(params.userParameters());

        d->permissions = TumorQueryUtils::instance()->getPermissions(params.databaseName, d->userName);

        LoginInfoWidget::instance()->logInUpdate(d->userName);
        emit signalLoginStateChanged();
        d->isLoggedIn = true;

        return true;
    }
    else
    {
        QMessageBox::critical(0, tr("Datenbankverbindung nicht möglich"),
                              tr("Die Zugangsdaten oder Verbindungseinstellungen zur Datenbank sind nicht korrekt."));
        return false;
    }
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
    d->permissions.clear();
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

bool UserInformation::hasKey(const QString& keyName)
{
    return !d->decryptionKey.value(keyName).isEmpty();
}

bool UserInformation::loadKeys()
{
    UserDetails details = TumorQueryUtils::instance()->retrieveUser(d->userName, d->password);
    if(details.id == -1)
        return false;
    d->decryptionKey = details.decryptionKeys;

    return true;
}


QString UserInformation::retrieveKey(const QString& keyName)
{
    return d->decryptionKey.value(keyName);
}

int UserInformation::retrievePermission(const QString& tableName)
{
    return d->permissions.value(tableName, AbstractQueryUtils::PERMISSION_NONE);
}

void UserInformation::setUsername(const QString& username)
{
    d->userName = username;
}

void UserInformation::setPassword(const QString& password)
{
    d->password = password;
}

QString UserInformation::username() const
{
    return d->userName;
}

QString UserInformation::password() const
{
    return d->password;
}


UserInformation::UserInformation()
    :d(new Private())
{
    QMutexLocker(&d->mutex);
    d->encryptionEnabled = EncryptionSettings::isEncryptionEnabled();
}

int UserInformation::checkDbConnection(const QString& username, const QString& password)
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
        return settingsDialog->exec();
    }
    else
    {
        return UserInformation::ALL_OK;
    }
}
