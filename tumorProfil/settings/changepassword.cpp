#include "changepassword.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>

class ChangePassword::Private
{
public:
    Private()
    {

    }
    QLabel *   mainLabel;
    QLineEdit* username;
    QLineEdit* oldPassword;
    QLineEdit* newPassword;
    QLineEdit* newPassword2;
    QPushButton* changePassword;
};
ChangePassword::ChangePassword(QWidget* parent)
    :QWidget(parent), d(new Private())
{
    setupUi();
}

void ChangePassword::setupUi()
{

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    d->mainLabel = new QLabel();
    d->mainLabel->setText(tr("Change user password from TumorProfil. \n"
                             "Please provide the old an new password. \n"
                             "The encryption keys will also be updated"));
    d->mainLabel->setAlignment(Qt::AlignCenter);
    QLabel* const usernameLabel     = new QLabel(tr("Username"));
    d->username                     = new QLineEdit();
    QLabel* const oldPasswordLabel  = new QLabel(tr("Old Password"));
    d->oldPassword                  = new QLineEdit();
    QLabel* const newPasswordLabel  = new QLabel(tr("New Password"));
    d->newPassword                         = new QLineEdit();
    QLabel* const newPasswordLabel2  = new QLabel(tr("Repeat New Password"));

    d->newPassword2                = new QLineEdit();

    d->changePassword               = new QPushButton(tr("Change Password"));

    QFormLayout* const expertSettinglayout           = new QFormLayout();

    expertSettinglayout->addRow(usernameLabel, d->username);
    expertSettinglayout->addRow(oldPasswordLabel, d->oldPassword);
    expertSettinglayout->addRow(newPasswordLabel, d->newPassword);
    expertSettinglayout->addRow(newPasswordLabel2, d->newPassword2);
    expertSettinglayout->addWidget(d->changePassword);

    mainLayout->addWidget(d->mainLabel);
    mainLayout->addLayout(expertSettinglayout);

}
