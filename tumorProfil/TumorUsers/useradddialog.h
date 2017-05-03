#ifndef USERADDDIALOG_H
#define USERADDDIALOG_H

#include <QDialog>
#include <QMap>

class QVBoxLayout;
class UserData{
public:
    QString userName;
    QString password;

    QList<QString> keys;
    QMap<QString, QString> privileges;
};

class UserAddDialog : public QDialog
{
public:
    UserAddDialog( UserData& data, bool isAdmin = false, bool login = false, bool exists = false);

    static UserData AddUser(bool isAdmin);
    static UserData editUser(bool isAdmin, UserData &data);
    static UserData login(bool isAdmin);

    QString username();
    QString password();
    QList<QString> selectedKeys();

    QMap<QString, QString> getUserPermissions() const;

public slots:

    void accept();

private:
    void setupUi(UserData& data, bool isAdmin, bool login, bool exists);
    void populateKeyList(UserData& data);
    QVBoxLayout* makePermissionLayout(QString userName);
    class Private;
    Private* d;
};

#endif // USERADDDIALOG_H
