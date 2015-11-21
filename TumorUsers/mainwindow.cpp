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

#include "databaseconfigelement.h"
#include "databaseguioptions.h"
#include "userwidget.h"
#include "useradddialog.h"
#include "queryutils.h"
#include "adminuser.h"
#include "mymessagebox.h"


class MainWindow::Private
{
public:
    Private()
    {

    }

    QToolBar* toolBar;
    UserWidget* userWidget;
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
    }
}

void MainWindow::setupUi()
{
    QWidget* widget = new QWidget(this);
    QHBoxLayout* hbox = new QHBoxLayout(widget);
    d->userWidget         = new UserWidget(widget);
    DatabaseGuiOptions* dbGui = new DatabaseGuiOptions(widget);

    hbox->addWidget(dbGui,2);
    hbox->addWidget(d->userWidget,6);
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

