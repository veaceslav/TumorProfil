#ifndef AUTHENTICATIONWINDOW_H
#define AUTHENTICATIONWINDOW_H

#include <QDialog>


class UserData
{
public:
    QString username;
    QString password;
    bool    success;
};

class AuthenticationWindow : public QDialog
{
    Q_OBJECT
public:
    AuthenticationWindow(const QString& username, QWidget* parent = 0);

    static UserData logIn(const QString& username = QString());

    QString username();
    QString password();
private:
    void setupUi();

    class Private;
    Private *d;
};

#endif // AUTHENTICATIONWINDOW_H
