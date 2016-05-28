#ifndef USERADDDIALOG_H
#define USERADDDIALOG_H

#include <QDialog>

class QVBoxLayout;
class UserData{
public:
    QString userName;
    QString password;

    QList<QString> keys;
};

class UserAddDialog : public QDialog
{
public:
    UserAddDialog( UserData& data, bool isAdmin = false, bool login = false);

    static UserData AddUser(bool isAdmin);
    static UserData editUser(bool isAdmin, UserData &data);
    static UserData login(bool isAdmin);

    QString username();
    QString password();
    QList<QString> selectedKeys();

public slots:
    void accept();
private:

    void setupUi(UserData& data, bool isAdmin, bool login);
    void populateKeyList(UserData& data);
    QVBoxLayout* makePermissionLayout();
    class Private;
    Private* d;
};

#endif // USERADDDIALOG_H
