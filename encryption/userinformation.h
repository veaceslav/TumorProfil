#ifndef USERINFORMATION_H
#define USERINFORMATION_H

#include <QPointer>
#include "authenticationwindow.h"

/**
 * @brief The UserInformation class is the singleton class which will store
 *        user decryption keys.
 */

class UserInformation : public QObject
{
    Q_OBJECT
public:

    enum LoginState
    {
        NOT_LOGGEDIN = 0,
        LOGGEDIN = 1
    };
    static UserInformation* instance();

    bool logIn();

    bool logOut();

    bool isEncryptionEnabled();

    bool isLoggedIn();

    LoginState toggleLogIn();

    bool hasKey(QString keyName);

    QString retrieveKey(QString keyName);

protected:
    static QPointer<UserInformation> internalPtr;
    UserInformation();

private:
    class Private;
    Private * d;
};

#endif // USERINFORMATION_H
