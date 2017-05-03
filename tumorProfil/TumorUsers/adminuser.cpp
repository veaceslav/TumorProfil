#include "adminuser.h"

#include <QMap>
#include <QVector>
#include <QDebug>
#include <QCryptographicHash>
#include <QMessageBox>
#include <QApplication>

#include "useradddialog.h"
#include "databaseaccess.h"
#include "aesutils.h"
#include "userqueryutils.h"

#include "tumoruserconstants.h"

QPointer<AdminUser> AdminUser::internalPtr = QPointer<AdminUser>();


class AdminUser::Private
{
public:
    Private()
    {
        isLoggedIn = false;
    }
    QString password;
    QMap<QString, MasterKey> masterKeys;
    QString aesFilling;
    QString adminSalt;
    bool    isLoggedIn;

    QString userDatabaseName;
    QString tumorProfilDatabaseName;
};

AdminUser* AdminUser::instance()
{
    if(AdminUser::internalPtr.isNull())
        AdminUser::internalPtr = new AdminUser();

    return AdminUser::internalPtr;
}

AdminUser::AdminUser() : d(new Private())
{

}

void AdminUser::loadKeys()
{
    d->masterKeys.clear();
    QVector<QVector<QVariant> > result = UserQueryUtils::instance()->retrieveMasterKeys(ADMIN_ID);

    foreach (QVector<QVariant> key, result)
    {
        QString decryptedKey = AesUtils::decryptMasterKey(d->password,
                                                            d->adminSalt,
                                                            key.at(MasterKey::VALUE_FIELD).toString());

        MasterKey mKey(key.at(MasterKey::NAME_FIELD).toString(),
                       decryptedKey);
        qDebug() << "Admin added key" << decryptedKey;
        d->masterKeys.insert(mKey.name, mKey);
    }
}

bool AdminUser::logIn()
{
    QString queryString = QString("SELECT * from Users WHERE name = \"%1\"").arg(ADMIN_NAME);
    QMap<QString, QVariant> bindMap;
    QVector<QVector<QVariant> > results;

    DatabaseAccess::instance()->executeDirectSql(queryString, bindMap, results);

    if(results.isEmpty())
    {
        return false;
    }

    while(true)
    {
        UserData data;
        if (DatabaseAccess::instance()->isOpen())
        {
            DatabaseParameters params = DatabaseAccess::instance()->databaseParams();
            if (params.userName == ADMIN_NAME)
            {
                data.userName = params.userName;
                data.password = params.password;
            }
        }
        if (data.userName.isEmpty() || data.password.isEmpty())
        {
            UserData data = UserAddDialog::login(true);
        }
        if (data.userName.isEmpty() || data.password.isEmpty())
            return false;

        // Compute password
        QString saltedPass(data.password + results.first().at(AdminUser::PASSWORD_SALT).toString());
        QByteArray passHash = QCryptographicHash::hash(saltedPass.toLatin1(), QCryptographicHash::Sha256);

        QByteArray storedHash = results.first().at(AdminUser::PASS_HASH).toByteArray();

        if(passHash != storedHash)
        {
            QMessageBox::StandardButton bt = QMessageBox::critical(qApp->activeWindow(), tr("Wrong Password"),
                                  tr("Wrong Password: Please try again"), QMessageBox::Retry | QMessageBox::Cancel);

            if(bt == QMessageBox::Cancel)
            {
                return false;
            }
        }
        else
        {
            d->isLoggedIn = true;
            d->password = data.password;
            d->adminSalt  = results.first().at(AdminUser::PASSWORD_SALT).toString();
            DatabaseParameters params = DatabaseAccess::instance()->databaseParams();
            d->tumorProfilDatabaseName = params.databaseName;
            d->userDatabaseName = params.databaseNameUsers;
            loadKeys();
            return true;
        }
    }
    return false;
}

QString AdminUser::masterKey(QString name)
{
    if(d->masterKeys.contains(name))
        return d->masterKeys.value(name).value;
    else
        return QString();
}

bool AdminUser::isLoggedIn()
{
    return d->isLoggedIn;
}

QString AdminUser::adminSalt()
{
    return d->adminSalt;
}

QString AdminUser::adminPassword()
{
    return d->password;
}

QList<QString> AdminUser::masterKeyNames()
{
    return d->masterKeys.keys();
}

QString AdminUser::userDatabaseName()
{
    return d->userDatabaseName;
}

QString AdminUser::tumorProfilDatabaseName()
{
    return d->tumorProfilDatabaseName;
}
