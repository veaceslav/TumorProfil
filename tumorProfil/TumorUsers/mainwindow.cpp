#include "mainwindow.h"

#include <QLineEdit>
#include <QString>
#include <QGroupBox>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QApplication>
#include <QMessageBox>
#include <QAction>

#include <QDesktopWidget>
#include <QToolBar>
#include <QHBoxLayout>
#include <QDebug>
#include <QTabWidget>

#include "databaseconfigelement.h"
#include "databaseguioptions.h"
#include "userwidget.h"
#include "useradddialog.h"
#include "userqueryutils.h"
#include "adminuser.h"
#include "mymessagebox.h"
#include "masterkeystable.h"
#include "addkeywidget.h"


#define ADMIN_ID 1

class MainWindow::Private
{
public:
    Private()
    {

    }

    QToolBar* toolBar;
    UserWidget* userWidget;
    MasterKeysTable* keysTables;
    DatabaseGuiOptions* dbGui;
};
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), d(new Private())
{
    setupUi();
    setupToolBar();

    connect(d->dbGui, SIGNAL(signalconnectedToDb()), this, SLOT(slotConnectedToDb()));
    resize(QDesktopWidget().availableGeometry(this).size() * 0.7);
}

MainWindow::~MainWindow()
{
}

void MainWindow::slotListUsers()
{

    AdminUser* admin = AdminUser::instance();

    bool result = true;
    if(!admin->isLoggedIn())
    {
        result = false;
        result = admin->logIn();
    }

    if(result)
    {
        d->userWidget->populateTable();
        d->keysTables->populateTable();
        slotEnableActions();
    }
}

void MainWindow::setupUi()
{
    QWidget* widget = new QWidget(this);
    QHBoxLayout* hbox = new QHBoxLayout(widget);
    QTabWidget* tabWidget = new QTabWidget(widget);

    d->userWidget         = new UserWidget(widget);
    d->keysTables         = new MasterKeysTable(widget);

    tabWidget->addTab(d->userWidget, tr("Users"));
    tabWidget->addTab(d->keysTables, tr("Keys"));
    d->dbGui = new DatabaseGuiOptions(widget);

    hbox->addWidget(d->dbGui,2);
    hbox->addWidget(tabWidget,6);
    this->setCentralWidget(widget);
}

void MainWindow::setupToolBar()
{
    d->toolBar = addToolBar(tr("Main"));


    // TODO: find nice icons later
    QAction* action = d->toolBar->addAction(QIcon::fromTheme(""),
                          tr("List Users"), this,
                          SLOT(slotListUsers()));

    action->setEnabled(false);

    action = d->toolBar->addAction(QIcon::fromTheme("add"),
                          tr("Add user"),
                          this, SLOT(slotAddUser()));

    action->setEnabled(false);

    action = d->toolBar->addAction(QIcon::fromTheme(""),
                          tr("Add Encryption Key"),
                          this,
                          SLOT(slotAddEncryptionKey()));

    action->setEnabled(false);

    action = d->toolBar->addAction(QIcon::fromTheme(""),
                          tr("Edit User"),
                          this,
                          SLOT(slotEditUser()));

    action->setEnabled(false);

    action = d->toolBar->addAction(QIcon::fromTheme(""),
                          tr("Delete Selected User"),
                          this,
                          SLOT(slotDeleteUser()));

    action->setEnabled(false);
    action = d->toolBar->addAction(QIcon::fromTheme(""),
                          tr("Delete Selected Key"),
                          this,
                          SLOT(slotDeleteMasterKey()));

    action->setEnabled(false);
}

bool MainWindow::slotAddUser()
{
    UserData data = UserAddDialog::AddUser(false);
    if(data.userName.isEmpty() || data.password.isEmpty())
        return false;

    UserDetails user = UserQueryUtils::instance()->addUser(data.userName, AbstractQueryUtils::USER, data.password);


    QMap<QString, QString>::iterator iter;
    for(iter=data.privileges.begin(); iter != data.privileges.end(); ++iter)
    {
        UserQueryUtils::instance()->grantMySqlPermissions(data.userName,
                                                          AdminUser::instance()->tumorProfilDatabaseName(),
                                                          QLatin1String("%"),
                                                          iter.key(),
                                                          iter.value());

        UserQueryUtils::instance()->grantMySqlPermissions(data.userName,
                                                          AdminUser::instance()->tumorProfilDatabaseName(),
                                                          QLatin1String("localhost"),
                                                          iter.key(),
                                                          iter.value());
    }

    UserQueryUtils::instance()->grantMySqlPermissions(data.userName, AdminUser::instance()->userDatabaseName());
    UserQueryUtils::instance()->grantMySqlPermissions(data.userName,
                                                      AdminUser::instance()->userDatabaseName(),
                                                      QLatin1String("localhost"));
    if(user.id == -1)
        return false;
    else
    {
        d->userWidget->addRow(user.id);
    }


    foreach(QString keyName, data.keys)
    {
        UserQueryUtils::instance()->addMasterKey(keyName, user.id, data.password, user.userSalt,
                                 AdminUser::instance()->masterKey(keyName));
    }

    return true;
}

void MainWindow::slotAddEncryptionKey()
{
    KeyInfo info = AddKeyWidget::addKey();

    UserQueryUtils::instance()->addMasterKey(info.name,
                                    ADMIN_ID,
                                    AdminUser::instance()->adminPassword(),
                                    AdminUser::instance()->adminSalt());

    d->keysTables->populateTable();

    AdminUser::instance()->loadKeys();
}

void MainWindow::slotEditUser()
{
    QList<QVariant> userData = d->userWidget->selectedRowData();

    if(userData.isEmpty())
        return;

    UserData oldData;
    oldData.userName = userData.at(1).toString();

    QVector<QVector<QVariant> > userKeys = UserQueryUtils::instance()->retrieveMasterKeys(userData.first().toInt());

    foreach (QVector<QVariant> key, userKeys)
    {
        oldData.keys.append(key.at(MasterKey::NAME_FIELD).toString());
    }

    UserData data = UserAddDialog::editUser(false, oldData);

    if(data.userName.isEmpty() || data.password.isEmpty()){
        return;
    }

    UserDetails user = UserQueryUtils::instance()->editUser(data.userName, AbstractQueryUtils::USER,
                                            data.password, userData.at(UserWidget::USERID_COLUMN).toInt());

    if(user.id == -1)
    {
        qDebug() << "Returning....";
        return;
    }
    else
    {
        d->userWidget->populateTable();
    }

    // Update Permissions---------------------------------------------------------------

    UserQueryUtils::instance()->revokeAllPrivileges(data.userName);

    QMap<QString, QString>::iterator iter;
    for(iter=data.privileges.begin(); iter != data.privileges.end(); ++iter)
    {
        UserQueryUtils::instance()->grantMySqlPermissions(data.userName,
                                                          AdminUser::instance()->tumorProfilDatabaseName(),
                                                          QLatin1String("%"),
                                                          iter.key(),
                                                          iter.value());

        UserQueryUtils::instance()->grantMySqlPermissions(data.userName,
                                                          AdminUser::instance()->tumorProfilDatabaseName(),
                                                          QLatin1String("localhost"),
                                                          iter.key(),
                                                          iter.value());
    }

    UserQueryUtils::instance()->grantMySqlPermissions(data.userName, AdminUser::instance()->userDatabaseName());
    UserQueryUtils::instance()->grantMySqlPermissions(data.userName,
                                                      AdminUser::instance()->userDatabaseName(),
                                                      QLatin1String("localhost"));

    // Update Master Keys-------------------------------------------------------------------
    UserQueryUtils::instance()->removeAllMasterKeys(user.id);

    QMap<QString, QString> userKeysUpdated;

    foreach(QString keyName, data.keys)
    {
        userKeysUpdated.insert(keyName,AdminUser::instance()->masterKey(keyName));
    }

    UserQueryUtils::instance()->updateUserMasterKeys(user.id,data.password,user.userSalt,userKeysUpdated);

    if(user.id == ADMIN_ID)
    {
        AdminUser::instance()->loadKeys();
        d->keysTables->populateTable();
    }

}

void MainWindow::slotDeleteUser()
{
    int index = d->userWidget->selectedRowId();

    if(index == -1)
    {
        MyMessageBox::showInfo(tr("Delete User"),
                               tr("No use is selected, Please select one user"));
        return;
    }

    if(index == 1)
    {
        MyMessageBox::showInfo(tr("Delete User"),
                               tr("Deleting admin user is not implemented"));
        return;
    }

    QList<QVariant> data = d->userWidget->selectedRowData();
    UserQueryUtils::instance()->removeUser(index,data.at(1).toString());

    d->userWidget->populateTable();
}

void MainWindow::slotDeleteMasterKey()
{
    QString keyName = d->keysTables->selectedRowName();

    if(keyName.isEmpty())
    {
        MyMessageBox::showInfo(tr("Delete Master Key"),
                               tr("No Key is selected, Please select one Key"));
        return;
    }


    UserQueryUtils::instance()->removeMasterKey(keyName);

    d->keysTables->populateTable();
}

void MainWindow::slotConnectedToDb()
{
    d->toolBar->actions().first()->setEnabled(true);
}

void MainWindow::slotEnableActions()
{
    QList<QAction*> actions = d->toolBar->actions();

    foreach(QAction* action, actions)
    {
        action->setEnabled(true);
    }
}

