#ifndef USERINFORMATION_H
#define USERINFORMATION_H

#include <QPointer>
#include "encryption/authenticationwindow.h"

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

    enum OperationStatus{
        ALL_OK = -2
    };
    static UserInformation* instance();

    bool logIn();

    bool logOut();

    bool isEncryptionEnabled();

    bool isLoggedIn();

    void setEncryptionEnabled(bool value);

    bool hasKey(const QString& keyName);

    bool loadKeys();

    /**
     * @brief retrieveKey - retrieve encryption key for database field
     * @param keyName     - keyname correspond to database field name
     * @return            - Encrption key, or QString() if use does not have the key
     */
    QString retrieveKey(const QString& keyName);

    int retrievePermission(const QString& tableName);

    void setUsername(const QString& username);

    void setPassword(const QString& password);

    QString username() const;

    QString password() const;

public slots:
    LoginState toggleLogIn();

signals:
    void signalLoginStateChanged();

protected:
    static QPointer<UserInformation> internalPtr;
    UserInformation();

private:

    int checkDbConnection(const QString& username, const QString& password);
    class Private;
    Private * d;
};

#endif // USERINFORMATION_H
