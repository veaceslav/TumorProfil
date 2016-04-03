#include "dbuserinformation.h"

#include "encryption/authenticationwindow.h"
#include "storage/databaseparameters.h"
#include "settings/databasesettings.h"
#include "settings/mainsettings.h"


QPointer<DbUserInformation> DbUserInformation::internalPtr = QPointer<DbUserInformation>();



class DbUserInformation::Private
{
public:
    Private()
    {
        loggedIn = false;
    }
    QString username;
    QString password;

    bool loggedIn;

};

DbUserInformation::DbUserInformation(QObject *parent)
    : QObject(parent), d(new Private())
{

}

DbUserInformation *DbUserInformation::instance()
{
    if(DbUserInformation::internalPtr.isNull())
        DbUserInformation::internalPtr = new DbUserInformation();

    return DbUserInformation::internalPtr;
}

bool DbUserInformation::loginDbUser()
{

    DatabaseParameters params;
    params.readFromConfig();
    UserData data = AuthenticationWindow::logIn(params.userName);

    params.userName = data.username;
    params.password = data.password;

    bool result = DatabaseSettings::checkDatabaseConnection(params);

    if(!result)
    {
        /** Db Options only **/
        MainSettings* settingsDialog = new MainSettings(true);
        settingsDialog->exec();
    } else {
        d->username = data.username;
        d->password = data.password;
        d->loggedIn = true;
        return true;
    }
}

QString DbUserInformation::dbUsername()
{
    return d->username;
}

QString DbUserInformation::dbPassword()
{
    return d->password;
}

void DbUserInformation::setUsername(QString username)
{
    d->username = username;
}

void DbUserInformation::setPassword(QString password)
{
    d->password = password;
}


