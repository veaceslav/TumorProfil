#ifndef AUTHENTICATIONWINDOW_H
#define AUTHENTICATIONWINDOW_H

#include <QDialog>


class UserData
{
public:
    QString username;
    QString password;
};

class AuthenticationWindow : public QDialog
{
    Q_OBJECT
public:
    AuthenticationWindow(QWidget* parent = 0);

    static UserData logIn();

    QString username();
    QString password();
private:
    void setupUi();

    class Private;
    Private *d;
};

#endif // AUTHENTICATIONWINDOW_H
