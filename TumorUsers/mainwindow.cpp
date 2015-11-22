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
};
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), d(new Private())
{
    setupUi();
    setupToolBar();
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
    DatabaseGuiOptions* dbGui = new DatabaseGuiOptions(widget);

    hbox->addWidget(dbGui,2);
    hbox->addWidget(tabWidget,6);
    this->setCentralWidget(widget);
}

void MainWindow::setupToolBar()
{
    d->toolBar = addToolBar(tr("Main"));


    // TODO: find nice icons later
    d->toolBar->addAction(QIcon::fromTheme(""),
                          tr("List Users"), this,
                          SLOT(slotListUsers()));

    d->toolBar->addAction(QIcon::fromTheme("add"),
                          tr("Add user"),
                          this, SLOT(slotAddUser()));

    d->toolBar->addAction(QIcon::fromTheme(""),
                          tr("Add Encryption Key"),
                          this,
                          SLOT(slotAddEncryptionKey()));

    d->toolBar->addAction(QIcon::fromTheme(""),
                          tr("Edit User"),
                          this,
                          SLOT(slotEditUser()));

    d->toolBar->addAction(QIcon::fromTheme(""),
                          tr("Delete Selected User"),
                          this,
                          SLOT(slotDeleteUser()));

}

bool MainWindow::slotAddUser()
{
    UserData data = UserAddDialog::AddUser(false);
    if(data.userName.isEmpty() || data.password.isEmpty())
        return false;
    qlonglong id = -1;
    id = QueryUtils::addUser(data.userName, QueryUtils::USER, data.password , AdminUser::instance()->masterKey());

    if(id == -1)
        return false;
    else
    {
        d->userWidget->addRow(id);
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

    QueryUtils::removeUser(index);

    d->userWidget->populateTable();
}

