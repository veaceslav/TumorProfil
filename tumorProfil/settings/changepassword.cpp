#include "changepassword.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>

#include "encryption/queryutils.h"

#define CHANGE_PASSWORD_TUMORPROFIL "ChangePasswordTumorProfil"
#define CHANGE_PASSWORD_TUMORUSERS "ChangePasswordTumorUsers"

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
    QLabel*      errorLabel;
};
ChangePassword::ChangePassword(QWidget* parent)
    :QWidget(parent), d(new Private())
{
    setupUi();

    connect(d->changePassword, SIGNAL(clicked(bool)),
            this, SLOT(slotChangePassword()));
}

void ChangePassword::slotChangePassword()
{

    if(d->newPassword->text().compare(d->newPassword2->text()))
    {
        setError(tr("Error: New Passwords do not match"));
        return;
    }
    DatabaseParameters params;
    params.readFromConfig();
    params.userName = d->username->text();
    params.password = d->oldPassword->text();


    bool result = TumorQueryUtils::instance()->openConnection(params, CHANGE_PASSWORD_TUMORPROFIL,
                                             TumorQueryUtils::TUMORPROFIL);

    if(!result)
    {
        setError(tr("Error: Could not open database connection to Tumorprofil.\n"
                    "Check database connection parameters"));
        return;
    }

    result = TumorQueryUtils::instance()->openConnection(params, CHANGE_PASSWORD_TUMORUSERS,
                                        TumorQueryUtils::TUMORUSER);

    if(!result)
    {
        setError(tr("Error: Could not open database connection to Tumorusers.\n"
                    "Check database connection parameters"));

        TumorQueryUtils::instance()->closeConnection(CHANGE_PASSWORD_TUMORPROFIL);
        return;
    }

    changeMySQLPassword();

    TumorQueryUtils::instance()->closeConnection(CHANGE_PASSWORD_TUMORPROFIL);
    TumorQueryUtils::instance()->closeConnection(CHANGE_PASSWORD_TUMORPROFIL);
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

    d->errorLabel = new QLabel();

    QPalette palette = d->errorLabel->palette();
    palette.setColor(d->errorLabel->backgroundRole(), Qt::red);
    palette.setColor(d->errorLabel->foregroundRole(), Qt::red);
    d->errorLabel->setPalette(palette);
    d->errorLabel->hide();
    d->errorLabel->setWordWrap(true);

    QFormLayout* const expertSettinglayout           = new QFormLayout();

    expertSettinglayout->addRow(usernameLabel, d->username);
    expertSettinglayout->addRow(oldPasswordLabel, d->oldPassword);
    expertSettinglayout->addRow(newPasswordLabel, d->newPassword);
    expertSettinglayout->addRow(newPasswordLabel2, d->newPassword2);
    expertSettinglayout->addWidget(d->changePassword);

    mainLayout->addWidget(d->mainLabel);
    mainLayout->addLayout(expertSettinglayout);
    mainLayout->addWidget(d->errorLabel);

}

void ChangePassword::setError(QString message)
{
    d->errorLabel->setText(message);
    d->errorLabel->show();
}

bool ChangePassword::changeMySQLPassword()
{
    QMap<QString, QVariant> bindValues;
    QVector<QVector<QVariant> > results;

    QString query = QString("SET PASSWORD = PASSWORD('%1')").arg(d->newPassword->text());

    return TumorQueryUtils::instance()->executeDirectSql(query,
                                 bindValues,
                                 results,
                                 QString(CHANGE_PASSWORD_TUMORPROFIL));
}

bool ChangePassword::updateEncryptionKeys()
{
    UserDetails userInfo = TumorQueryUtils::instance()->retrieveUser(d->username->text(), d->oldPassword->text());

}
