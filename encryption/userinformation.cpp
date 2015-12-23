#include "userinformation.h"

#include <QTimer>
#include <QMap>

QPointer<UserInformation> UserInformation::internalPtr = QPointer<UserInformation>();


class UserInformation::Private
{
public:
    Private();

    bool isLoggedIn;
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

UserInformation::UserInformation()
{

}
