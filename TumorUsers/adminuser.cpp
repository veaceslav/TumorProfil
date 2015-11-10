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

QPointer<AdminUser> AdminUser::internalPtr = QPointer<AdminUser>();


class AdminUser::Private
{
public:
    Private()
    {
        isLoggedIn = false;
    }
    QString password;
    QString masterKey;
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
    d->password = password;

    QByteArray masterKeyHash = queryResult.first().at(AdminUser::ENCRYPTED_KEY).toByteArray();

    QString filling = queryResult.first().at(AdminUser::AES_FILLING).toString();

    d->masterKey = QueryUtils::decryptMasterKey(d->password, filling, masterKeyHash);

    qDebug() << "Master key is: " << d->masterKey;
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
        UserData data = UserAddDialog::AddUser(true);
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
    return d->masterKey;
}
