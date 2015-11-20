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
    QList<MasterKey> masterKeys;
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

void AdminUser::setData(QString password, QVector<QVector<QVariant> > &queryResult)
{
    Q_UNUSED(queryResult);
    d->password = password;

    QVector<QVector<QVariant> > result = QueryUtils::retrieveMasterKeys(ADMIN_ID);

    foreach (QVector<QVariant> key, result)
    {
        MasterKey mKey(key.at(MasterKey::NAME_FIELD).toString(),
                       key.at(MasterKey::VALUE_FIELD).toString());
        d->masterKeys.append(mKey);
    }

    //QByteArray masterKeyHash = queryResult.first().at(AdminUser::ENCRYPTED_KEY).toByteArray();

//    QString filling = queryResult.first().at(AdminUser::AES_FILLING).toString();

//    d->masterKey = QueryUtils::decryptMasterKey(d->password, filling, masterKeyHash);

//    qDebug() << "Master key is: " << d->masterKey;
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
            setData(data.password, results);
            return true;
        }
    }
    return true;
}

QString AdminUser::masterKey()
{
    if(d->masterKeys.isEmpty())
        return QString();
    return d->masterKeys.first().value;
}
