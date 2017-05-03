#include "authenticationwindow.h"

#include <QDialogButtonBox>
#include <QLineEdit>
#include <QLabel>
#include <QListWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QSettings>

class AuthenticationWindow::Private
{
public:
    Private()
    {

    }
    QDialogButtonBox*   buttons;
    QLineEdit*      username;
    QLineEdit*      password;
    QLabel*         errMsg;
    QLabel*         mainLabel;
    bool            login;
    QPushButton*    skipButton;
};
AuthenticationWindow::AuthenticationWindow(QString username, QWidget *parent)
    :QDialog(parent), d(new Private())
{
    setModal(true);

    d->buttons = new QDialogButtonBox(this);
    d->skipButton = d->buttons->addButton(tr("Skip"), QDialogButtonBox::NoRole);
    d->buttons->addButton(QDialogButtonBox::Ok);
    d->buttons->addButton(QDialogButtonBox::Cancel);


    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);

    setupUi();

    d->username->setText(username);

    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(d->skipButton, SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(reject()));
}

UserData AuthenticationWindow::logIn(QString username)
{
    AuthenticationWindow* auth = new AuthenticationWindow(username);

    int result = auth->exec();

    UserData data;
    if(result)
    {
        data.username = auth->username();
        data.password = auth->password();
        data.success  = true;
    }

    return data;
}


QString AuthenticationWindow::username()
{
    return d->username->text();
}

QString AuthenticationWindow::password()
{
    return d->password->text();
}

void AuthenticationWindow::setupUi()
{
    d->mainLabel = new QLabel(this);
    d->mainLabel->setWordWrap(true);

    d->mainLabel->setText(tr("Please provide the Username and Password"));


    QGridLayout* gLayout = new QGridLayout();

    QLabel* userLabel = new QLabel(this);
    userLabel->setText(tr("Username:"));
    d->username = new QLineEdit(this);


    QLabel* passwordLabel = new QLabel(this);
    passwordLabel->setText(tr("Password:"));
    d->password = new QLineEdit(this);
    d->password->setEchoMode(QLineEdit::Password);

    d->errMsg = new QLabel(this);

    gLayout->addWidget(d->mainLabel, 1, 1, 1, 4);
    gLayout->addWidget(userLabel, 2, 1, 1, 2);
    gLayout->addWidget(d->username, 2, 3, 1, 2);
    gLayout->addWidget(passwordLabel, 3, 1, 1, 2);
    gLayout->addWidget(d->password, 3, 3, 1, 2);

    gLayout->addWidget(d->errMsg, 4, 1, 1, 4);

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    QHBoxLayout* const hbx = new QHBoxLayout(this);

    hbx->addLayout(gLayout);

    vbx->addLayout(hbx);
    vbx->addWidget(d->buttons);

    d->username->setFocus();
}
