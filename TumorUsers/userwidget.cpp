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
    QStringList headerLabelsList;

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
}

void UserWidget::setTableHeader()
{
    QStringList lst;

    d->headerLabelsList << tr("Number")
                        << tr("Name")
                        << tr("Group")
                        << tr("Password Hash")
                        << tr("Salt")
                        << tr("Encrypted Key");

    d->tableView->setHorizontalHeaderLabels(d->headerLabelsList);
}

