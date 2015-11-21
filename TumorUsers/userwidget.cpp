#include "userwidget.h"

#include <QTableWidget>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QDebug>
#include <QHeaderView>

#include "databaseaccess.h"

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
}

UserWidget::~UserWidget()
{

}

void UserWidget::populateTable()
{
    d->tableView->clear();
    setTableHeader();
    QLatin1String queryString("SELECT * from Users");
    QMap<QString, QVariant> bindMap;
    QVector<QVector<QVariant> > results;

    DatabaseAccess::instance()->executeDirectSql(queryString, bindMap, results);

    if(results.isEmpty())
    {
        return;
    }
    d->tableView->setRowCount(results.size());

    for(int i = 0 ; i < results.size(); i++)
    {
        QVector<QVariant> item = results.at(i);
        for(int j = 0; j < item.size(); j++)
        {

            QVariant cellData = item.at(j);


            QTableWidgetItem* tableItem = 0;
            if(cellData.type() == QVariant::Int)
            {
                tableItem = new QTableWidgetItem(QString::number(cellData.toInt()));
            }
            if(cellData.type() == QVariant::String)
            {
                tableItem = new QTableWidgetItem(cellData.toString());
            }
            if(cellData.type() == QVariant::ByteArray)
            {
                tableItem = new QTableWidgetItem(QString(cellData.toByteArray().toBase64()));
            }
            if(tableItem == NULL)
                qDebug() << "Element is NULL" << cellData.type();

            d->tableView->setItem(i,j,tableItem);
        }

    }


}

void UserWidget::addRow(qlonglong id)
{
    QLatin1String queryString("SELECT * from Users WHERE id = :id");
    QMap<QString, QVariant> bindMap;
    QVector<QVector<QVariant> > results;

    bindMap[QLatin1String(":id")] = id;

    DatabaseAccess::instance()->executeDirectSql(queryString, bindMap, results);

    if(results.isEmpty())
    {
        qDebug() << "No rows for given id";
        return;
    }

    int rowcount = d->tableView->rowCount();
    d->tableView->setRowCount(rowcount+1);

    QVector<QVariant> item = results.first();
    for(int j = 0; j < item.size(); j++)
    {

        QVariant cellData = item.at(j);


        QTableWidgetItem* tableItem = 0;
        if(cellData.type() == QVariant::Int)
        {
            tableItem = new QTableWidgetItem(QString::number(cellData.toInt()));
        }
        if(cellData.type() == QVariant::String)
        {
            tableItem = new QTableWidgetItem(cellData.toString());
        }
        if(cellData.type() == QVariant::ByteArray)
        {
            tableItem = new QTableWidgetItem(QString(cellData.toByteArray().toBase64()));
        }
        if(tableItem == NULL)
            qDebug() << "Element is NULL" << cellData.type();

        d->tableView->setItem(rowcount,j,tableItem);
    }
}

int UserWidget::selectedRowId()
{

    QList<QTableWidgetItem*> items = d->tableView->selectedItems();

    if(items.isEmpty())
        return -1;

    int rowNr = items.first()->row();

    QTableWidgetItem* item = d->tableView->item(rowNr, 0);

    return item->text().toInt();
}

void UserWidget::setupUi()
{
    QVBoxLayout* lay = new QVBoxLayout(this);
    d->tableView = new QTableWidget(this);
    d->tableView->setWordWrap(true);
    setTableHeader();
    lay->addWidget(d->tableView);
}

void UserWidget::setTableHeader()
{
    QHeaderView* header = d->tableView->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);

    if(d->headerLabelsList.isEmpty())
    {
        d->headerLabelsList << tr("User Id")
                            << tr("Name")
                            << tr("Group")
                            << tr("Salt")
                            << tr("Password Hash")
                            << tr("Private key filling");
    }
    d->tableView->setColumnCount(d->headerLabelsList.size());
    d->tableView->setHorizontalHeaderLabels(d->headerLabelsList);
}

