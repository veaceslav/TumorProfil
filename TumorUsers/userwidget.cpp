#include "userwidget.h"

#include <QTableWidget>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QVBoxLayout>

class UserWidget::Private
{
public:
    Private()
    {

    }
    QTableWidget* tableView;
    QStandardItemModel* tableModel;

};

UserWidget::UserWidget(QWidget *parent)
    : QWidget(parent), d(new Private())
{
    setupUi();
    setTableHeader();
}

UserWidget::~UserWidget()
{

}

void UserWidget::setupUi()
{
    QVBoxLayout* lay = new QVBoxLayout(this);
    d->tableView = new QTableWidget(this);
    d->tableView->setColumnCount(5);
    lay->addWidget(d->tableView);
//    d->tableModel = new QStandardItemModel(this);
    //    d->tableView->setModel(d->tableModel);
}

void UserWidget::setTableHeader()
{
    QStringList lst;

    lst << "Name"
        << "Group"
        << "Password Hash"
        << "Salt"
        << "Encrypted Key";
    d->tableView->setHorizontalHeaderLabels(lst);


}

