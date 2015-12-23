#ifndef USERINFORMATION_H
#define USERINFORMATION_H

#include <QPointer>

/**
 * @brief The UserInformation class is the singleton class which will store
 *        user decryption keys.
 */

class UserInformation : public QObject
{
    Q_OBJECT
public:
    static UserInformation* instance();

protected:
    static QPointer<UserInformation> internalPtr;
    UserInformation();

private:
    class Private;
    Private * d;
};

#endif // USERINFORMATION_H
