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
#include "queryutils.h"
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
    qDebug() << "Connection is established";

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

    UserDetails user = QueryUtils::addUser(data.userName, QueryUtils::USER, data.password);

    if(user.id == -1)
        return false;
    else
    {
        d->userWidget->addRow(user.id);
    }


    foreach(QString keyName, data.keys)
    {
        QueryUtils::addMasterKey(keyName, user.id, data.password, user.aesFilling,
                                 AdminUser::instance()->masterKey(keyName));
    }

    return true;
}

void MainWindow::slotAddEncryptionKey()
{
    KeyInfo info = AddKeyWidget::addKey();

    QueryUtils::addMasterKey(info.name,
                             ADMIN_ID,
                             AdminUser::instance()->adminPassword(),
                             AdminUser::instance()->aesFilling());

    d->keysTables->populateTable();

    AdminUser::instance()->loadKeys();
}

void MainWindow::slotEditUser()
{

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

    QueryUtils::removeUser(index);

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


    QueryUtils::removeMasterKey(keyName);

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

