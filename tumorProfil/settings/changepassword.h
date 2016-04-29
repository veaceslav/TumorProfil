#ifndef CHANGEPASSWORD_H
#define CHANGEPASSWORD_H

#include <QWidget>

class ChangePassword : public QWidget
{
    Q_OBJECT
public:
    ChangePassword(QWidget *parent = 0);


private slots:
    void slotChangePassword();

private:

    void setupUi();

    void setError(QString message);

    void setMessage(QString message);

    bool changeMySQLPassword();

    bool updateEncryptionKeys();

    class Private;
    Private* const d;
};

#endif // CHANGEPASSWORD_H
