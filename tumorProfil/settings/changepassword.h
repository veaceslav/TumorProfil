#ifndef CHANGEPASSWORD_H
#define CHANGEPASSWORD_H

#include <QWidget>

class ChangePassword : public QWidget
{
    Q_OBJECT
public:
    ChangePassword(QWidget *parent = 0);

private:

    void setupUi();

    class Private;
    Private* const d;
};

#endif // CHANGEPASSWORD_H
