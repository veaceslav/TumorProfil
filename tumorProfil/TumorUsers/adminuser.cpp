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
#include "queryutils.h"

#define ADMIN_ID 1

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
    bool    isLoggedIn;
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
    QVector<QVector<QVariant> > result = QueryUtils::retrieveMasterKeys(ADMIN_ID);

    foreach (QVector<QVariant> key, result)
    {
        QString decryptedKey = AesUtils::decryptMasterKey(d->password,
                                                            d->aesFilling,
                                                            key.at(MasterKey::VALUE_FIELD).toString());

        MasterKey mKey(key.at(MasterKey::NAME_FIELD).toString(),
                       decryptedKey);
        qDebug() << "Admin added key" << decryptedKey;
        d->masterKeys.insert(mKey.name, mKey);
    }
}

bool AdminUser::logIn()
{
    QLatin1String queryString("SELECT * from Users WHERE name = \"admin\"");
    QMap<QString, QVariant> bindMap;
    QVector<QVector<QVariant> > results;

    DatabaseAccess::instance()->executeDirectSql(queryString, bindMap, results);

    if(results.isEmpty())
    {
        return false;
    }

    while(true)
    {
        UserData data = UserAddDialog::login(true);
        if(data.userName.isEmpty() || data.password.isEmpty())
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
            d->aesFilling = results.first().at(AdminUser::AES_FILLING).toString();
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

QString AdminUser::aesFilling()
{
    return d->aesFilling;
}

QString AdminUser::adminPassword()
{
    return d->password;
}

QList<QString> AdminUser::masterKeyNames()
{
    return d->masterKeys.keys();
}
