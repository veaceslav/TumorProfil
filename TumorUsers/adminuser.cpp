#include "adminuser.h"
#include "useradddialog.h"
#include "databaseaccess.h"

#include <QMap>
#include <QVector>
#include <QDebug>
#include <QCryptographicHash>
#include <QMessageBox>
#include <QApplication>

QPointer<AdminUser> AdminUser::internalPtr = QPointer<AdminUser>();


class AdminUser::Private
{
public:
    Private()
    {
        isLoggedIn = false;
    }
    QString password;
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
        QString saltedPass(data.password + results.first().at(3).toString());
        QByteArray passHash = QCryptographicHash::hash(saltedPass.toLatin1(), QCryptographicHash::Sha256);

        QByteArray storedHash = results.first().at(4).toByteArray();

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
            return true;
        }
    }
        return true;
}