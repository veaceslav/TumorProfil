#ifndef AUTHENTICATIONWINDOW_H
#define AUTHENTICATIONWINDOW_H

#include <QDialog>

class AuthenticationWindow : public QDialog
{
    Q_OBJECT
public:
    AuthenticationWindow(QWidget* parent = 0);

private:
    void setupUi();
};

#endif // AUTHENTICATIONWINDOW_H
