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

UserAddDialog::UserAddDialog(UserData &data, bool isAdmin, bool login) : QDialog(), d(new Private)
{

    setModal(true);
    d->login = login;

    d->buttons = new QDialogButtonBox(QDialogButtonBox::Ok   |
                                      QDialogButtonBox::Cancel, this);

    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);

    setupUi(data, isAdmin,login);

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

    if(result != -1)
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

QMap<QString, QString> UserAddDialog::getUserPermissions()
{
    QMap<QString, QString> privileges;
    QMap<QString, QButtonGroup*>::iterator it;
    for(it=d->permissions.begin(); it != d->permissions.end(); ++it)
    {
        switch(it.value()->checkedId())
        {
        case 0: // none, do not have any privileges for that table
            break;
        case 1:
            privileges.insert(it.key(), QLatin1String("SELECT"));
            break;
        case 2:
            privileges.insert(it.key(), QLatin1String("ALL"));
            break;
        default:
            qDebug() << "Error, we should not have anything except 0,1,2";
            break;
        }
    }

    return privileges;
}

void UserAddDialog::setupUi(UserData &data, bool isAdmin, bool login)
{
    d->mainLabel = new QLabel(this);
    d->mainLabel->setWordWrap(true);
    if(login)
        d->mainLabel->setText(tr("Please provide the Username and Password"));
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

void UserAddDialog::setPermissions(QString userName)
{



}


QVBoxLayout* UserAddDialog::makePermissionLayout(QString userName){
    QVBoxLayout* lay = new QVBoxLayout();

    QVector<QString> tableNames = UserQueryUtils::instance()->getTumorProfilTables(
                                    AdminUser::instance()->tumorProfilDatabaseName());

    if(tableNames.size() != TUMORPROFIL_TABLES_SIZE){
        QMessageBox::critical(this, "Table name size mismatch",
                                    "It seems that you have more tables in database than declared in tumoruserconstants.\n"
                                    "If you added new tables, then add them to tumoruserconstants also\n"
                                    "Please set TUMORPROFIL_TABLES and TUMORPROFIL_TABLES_SIZE with the correct data\n"
                                    "The new tables will not appear in the layout");
    }

    std::string data[] = TUMORPROFIL_TABLES;

    bool haveDefaults = !(userName.isEmpty());

    QMap<QString, int> perm;
    if(!userName.isEmpty()){
        QString permString = QString("'%1'@'localhost'").arg(userName);
        perm = UserQueryUtils::instance()->getPermissions(AdminUser::instance()->tumorProfilDatabaseName(),
                                                          permString);
    }

    for(int iter = 0; iter < TUMORPROFIL_TABLES_SIZE; iter++)
    {
        QLatin1String tableName(data[iter].data());
        QHBoxLayout* tmpLay = new QHBoxLayout();
        tmpLay->addWidget(new QLabel(tableName));
        QRadioButton* none = new QRadioButton("None");
        QRadioButton* read = new QRadioButton("Read");
        QRadioButton* readWrite = new QRadioButton("Read+Write");
        readWrite->setChecked(true);
        QButtonGroup* buttonGroup = new QButtonGroup(this);
        buttonGroup->addButton(none, 0);
        buttonGroup->addButton(read, 1);
        buttonGroup->addButton(readWrite, 2);
        tmpLay->addWidget(none);
        tmpLay->addWidget(read);
        tmpLay->addWidget(readWrite);

        if(haveDefaults){
            switch(perm[tableName]){
            case 0:
                none->setChecked(true);
                break;
            case 1:
                read->setChecked(true);
                break;
            case 2:
                readWrite->setChecked(true);
                break;
            default:
                break;
            }
        }

        d->permissions.insert(tableName,buttonGroup);


        lay->addLayout(tmpLay);
    }

    return lay;
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
