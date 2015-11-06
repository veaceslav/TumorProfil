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
    QLatin1String queryString("SELECT * from Users");
    QMap<QString, QVariant> bindMap;
    QVector<QVector<QVariant> > results;

    DatabaseAccess::instance()->executeDirectSql(queryString, bindMap, results);

    qDebug() << "results: " << results.isEmpty();
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
                qDebug() << cellData;
            }
            if(cellData.type() == QVariant::String)
            {
                tableItem = new QTableWidgetItem(cellData.toString());
                qDebug() << cellData;
            }
            if(cellData.type() == QVariant::ByteArray)
            {
                tableItem = new QTableWidgetItem(QString(cellData.toByteArray().toBase64()));
                qDebug() << cellData;
            }
            if(tableItem == NULL)
                qDebug() << "Element is NULL" << cellData.type();

            d->tableView->setItem(i,j,tableItem);
        }

    }


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

    d->headerLabelsList << tr("Number")
                        << tr("Name")
                        << tr("Group")
                        << tr("Salt")
                        << tr("Password Hash")
                        << tr("Private key filling")
                        << tr("Encrypted Key");

    d->tableView->setColumnCount(d->headerLabelsList.size());
    d->tableView->setHorizontalHeaderLabels(d->headerLabelsList);
}

