#include "useradddialog.h"
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QGridLayout>
#include <QListWidget>
#include <QDebug>
#include <QRadioButton>
#include <QButtonGroup>
#include <QMessageBox>
#include <string.h>

#include "userqueryutils.h"
#include "tumoruserconstants.h"

#include "adminuser.h"

class UserAddDialog::Private
{
public:
    Private()
    {

    }
    QDialogButtonBox*   buttons;
    QLineEdit*      username;
    QLineEdit*      password;
    QLineEdit*      passwordSecondTime;
    QLabel*         errMsg;
    QLabel*         mainLabel;
    QListWidget*    keyList;
    QMap<QString, QButtonGroup*> permissions;
    bool            login;

};

UserAddDialog::UserAddDialog(UserData &data, bool isAdmin, bool login, bool exists) : QDialog(), d(new Private)
{

    setModal(true);
    d->login = login;

    d->buttons = new QDialogButtonBox(QDialogButtonBox::Ok   |
                                      QDialogButtonBox::Cancel, this);

    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);

    setupUi(data, isAdmin, login, exists);

    populateKeyList(data);
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
        data.keys     = dlg->selectedKeys();

        data.privileges = dlg->getUserPermissions();
    }

    delete dlg;
    return data;
}

UserData UserAddDialog::editUser(bool isAdmin, UserData& data)
{
    UserAddDialog* dlg = new UserAddDialog(data, isAdmin, false);

    int result = dlg->exec();

    UserData returnData;

    if(result)
    {
        returnData.userName = dlg->username();
        returnData.password = dlg->password();
        returnData.keys     = dlg->selectedKeys();

        returnData.privileges = dlg->getUserPermissions();

    }
    delete dlg;
    return returnData;
}

UserData UserAddDialog::login(bool isAdmin)
{
    UserData data;
    UserAddDialog* dlg = new UserAddDialog(data, isAdmin, true);

    int result = dlg->exec();

    if(result)
    {
        data.userName = dlg->username();
        data.password = dlg->password();
    }

    delete dlg;
    return data;
}

QString UserAddDialog::username()
{
    return d->username->text();
}

QString UserAddDialog::password()
{
    return d->password->text();
}

QList<QString> UserAddDialog::selectedKeys()
{
    QList<QString> lst;

    for(int i = 0; i < d->keyList->count(); i++)
    {
        QListWidgetItem* item = d->keyList->item(i);
        if(item->checkState() == Qt::Checked)
            lst.append(item->text());
    }
    qDebug() << lst.size();
    return lst;
}

void UserAddDialog::accept()
{
    if(!d->login)
    {
        if(d->password->text() != d->passwordSecondTime->text())
        {
            d->errMsg->setText(tr("Password does not match"));
            return;
        }
    }
    QDialog::accept();
}

QMap<QString, QString> UserAddDialog::getUserPermissions() const
{
    QMap<QString, QString> privileges;
    QMap<QString, QButtonGroup*>::const_iterator it;
    for(it = d->permissions.begin(); it != d->permissions.end(); ++it)
    {
        switch(it.value()->checkedId())
        {
        case AbstractQueryUtils::PERMISSION_NONE: // none, do not have any privileges for that table
            break;
        case AbstractQueryUtils::PERMISSION_READ:
            privileges[it.key()] = "SELECT";
            break;
        case AbstractQueryUtils::PERMISSION_READWRITE:
            privileges[it.key()] = "ALL";
            break;
        default:
            break;
        }
    }

    return privileges;
}

void UserAddDialog::setupUi(UserData &data, bool isAdmin, bool login, bool exists)
{
    d->mainLabel = new QLabel(this);
    d->mainLabel->setWordWrap(true);
    if(login)
        d->mainLabel->setText(tr("Please provide the Username and Password"));
    else if (exists)
        d->mainLabel->setText(tr("Edit username and password of the user:"));
    else
        d->mainLabel->setText(tr("Choose username and password for the new user:"));

    QGridLayout* gLayout = new QGridLayout();

    QLabel* userLabel = new QLabel(this);
    userLabel->setText(tr("Username:"));
    d->username = new QLineEdit(this);

    if(isAdmin)
    {
        d->username->setText(tr("root"));
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
    gLayout->addWidget(d->username, 2, 3, 1, 2);
    gLayout->addWidget(passwordLabel, 3, 1, 1, 2);
    gLayout->addWidget(d->password, 3, 3, 1, 2);

    if(!login)
    {
        gLayout->addWidget(password2Label, 4, 1, 1, 2);
        gLayout->addWidget(d->passwordSecondTime, 4, 3, 1, 2);
    }
    else
    {
        password2Label->hide();
        d->passwordSecondTime->hide();
    }
    gLayout->addWidget(d->errMsg, 5, 1, 1, 4);

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    QHBoxLayout* const hbx = new QHBoxLayout(this);

    // List widget with keys
    d->keyList = new QListWidget();


    hbx->addLayout(gLayout);

    if(!login && !isAdmin)
    {
        hbx->addWidget(d->keyList);
    }

    vbx->addLayout(hbx);
    if(!login && !isAdmin)
    {
        vbx->addLayout(makePermissionLayout(data.userName));
    }

    if(!data.userName.isEmpty())
    {
        d->username->setText(data.userName);
    }

    vbx->addWidget(d->buttons);
}

QVBoxLayout* UserAddDialog::makePermissionLayout(QString userName)
{
    QVBoxLayout* vbox = new QVBoxLayout();

    QVector<QString> tableNames = UserQueryUtils::instance()->getTumorProfilTables(
                                    AdminUser::instance()->tumorProfilDatabaseName());
    QMap<QString, int> perms = UserQueryUtils::instance()->getPermissions(AdminUser::instance()->tumorProfilDatabaseName(), userName);
    qDebug() << "makePermissionLayout" << perms;
    int defaultPermission = AbstractQueryUtils::PERMISSION_NONE;
    if (userName == ADMIN_NAME)
    {
        defaultPermission = AbstractQueryUtils::PERMISSION_READWRITE;
    }

    foreach(const QString& tableName, tableNames)
    {
        QHBoxLayout* hbox = new QHBoxLayout();
        hbox->addWidget(new QLabel(tableName));
        QRadioButton* none = new QRadioButton(tr("Kein Zugriff"));
        QRadioButton* read = new QRadioButton(tr("Lesen"));
        QRadioButton* readWrite = new QRadioButton(tr("Lesen/Schreiben"));
        QButtonGroup* buttonGroup = new QButtonGroup(this);
        buttonGroup->addButton(none, AbstractQueryUtils::PERMISSION_NONE);
        buttonGroup->addButton(read, AbstractQueryUtils::PERMISSION_READ);
        buttonGroup->addButton(readWrite, AbstractQueryUtils::PERMISSION_READWRITE);
        buttonGroup->button(perms.value(tableName, defaultPermission))->setChecked(true);
        hbox->addWidget(none);
        hbox->addWidget(read);
        hbox->addWidget(readWrite);

        vbox->addLayout(hbox);
        d->permissions.insert(tableName,buttonGroup);
    }

    return vbox;
}

void UserAddDialog::populateKeyList(UserData &data)
{
    QList<QString> keyNames = AdminUser::instance()->masterKeyNames();

    foreach(QString keyName, keyNames)
    {
        QListWidgetItem* item = new QListWidgetItem(keyName, d->keyList);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
        if(data.keys.contains(keyName))
            item->setCheckState(Qt::Checked);
        else
            item->setCheckState(Qt::Unchecked); // AND initialize check state
    }
}
