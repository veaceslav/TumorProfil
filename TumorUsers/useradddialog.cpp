#include "useradddialog.h"
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QGridLayout>

class UserAddDialog::Private
{
public:
    Private()
    {

    }
    QDialogButtonBox* buttons;
    QLineEdit* username;
    QLineEdit* password;
    QLineEdit* passwordSecondTime;
    QLabel*    errMsg;
    QLabel*    mainLabel;
};

UserAddDialog::UserAddDialog(UserData &data, bool isAdmin, bool login) : QDialog(), d(new Private)
{

    setModal(true);

    d->buttons = new QDialogButtonBox(QDialogButtonBox::Ok   |
                                      QDialogButtonBox::Cancel, this);

    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);

    setupUi(isAdmin,login);

    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(reject()));


}

UserData UserAddDialog::AddUser(bool isAdmin)
{

    UserData data;
    UserAddDialog* dlg = new UserAddDialog(data, isAdmin, false);

    int result = dlg->exec();

    if(result)
    {
        data.userName = dlg->username();
        data.password = dlg->password();
    }

    delete dlg;
    return data;
}

UserData UserAddDialog::editUser(bool isAdmin, UserData& data)
{
    UserAddDialog* dlg = new UserAddDialog(data, isAdmin, false);

    int result = dlg->exec();

    UserData returnData;

    if(result != -1)
    {
        returnData.userName = dlg->username();
        returnData.password = dlg->password();
    }
    delete dlg;
    return returnData;
}

UserData UserAddDialog::login()
{

}

QString UserAddDialog::username()
{
    return d->username->text();
}

QString UserAddDialog::password()
{
    return d->password->text();
}

void UserAddDialog::setupUi(bool isAdmin, bool login)
{
    d->mainLabel = new QLabel(this);
    d->mainLabel->setWordWrap(true);
    d->mainLabel->setText(tr("Choose username and password for the new user:"));

    QGridLayout* gLayout = new QGridLayout();

    QLabel* userLabel = new QLabel(this);
    userLabel->setText(tr("Username:"));
    d->username = new QLineEdit(this);

    if(isAdmin)
    {
        d->username->setText(tr("admin"));
        d->username->setEnabled(false);
    }

    QLabel* passwordLabel = new QLabel(this);
    passwordLabel->setText(tr("Password:"));
    d->password = new QLineEdit(this);

    QLabel* password2Label = new QLabel(this);
    password2Label->setWordWrap(true);
    password2Label->setText(tr("Password one more time:"));
    d->passwordSecondTime = new QLineEdit(this);
    d->errMsg = new QLabel(this);

    gLayout->addWidget(d->mainLabel, 1, 1, 1, 4);
    gLayout->addWidget(userLabel, 2, 1, 1, 2);
    gLayout->addWidget(d->username, 2, 2, 1, 2);
    gLayout->addWidget(passwordLabel, 3, 1, 1, 2);
    gLayout->addWidget(d->password, 3, 2, 1, 2);
    if(!login)
    {
        gLayout->addWidget(password2Label, 4, 1, 1, 2);
        gLayout->addWidget(d->passwordSecondTime, 4, 2, 1, 2);
    }
    gLayout->addWidget(d->errMsg, 5, 1, 1, 4);

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addLayout(gLayout);
    vbx->addWidget(d->buttons);
}

