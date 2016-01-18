#ifndef LOGININFOWIDGET_H
#define LOGININFOWIDGET_H

#include <QWidget>
#include <QPointer>

class LoginInfoWidget : public QWidget
{
    Q_OBJECT
public:


    static LoginInfoWidget* instance();
    void logInUpdate(QString userName);
    void logOutUpdate();

protected:
    static QPointer<LoginInfoWidget> internalPtr;
    LoginInfoWidget();

private:
    class Private;
    Private * d;
};

#endif // LOGININFOWIDGET_H
