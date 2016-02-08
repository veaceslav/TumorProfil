#include "masterkeystable.h"


#include <QTableWidget>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QDebug>
#include <QHeaderView>

#include "databaseaccess.h"

class MasterKeysTable::Private
{
public:
    Private()
    {

    }
    QTableWidget* tableView;
    QStandardItemModel* tableModel;
    QStringList headerLabelsList;

};

MasterKeysTable::MasterKeysTable(QWidget *parent)
    : QWidget(parent), d(new Private())
{
    setupUi();
}


MasterKeysTable::~MasterKeysTable()
{
    delete d;
}

void MasterKeysTable::populateTable()
{
    d->tableView->clear();
    setTableHeader();
    QLatin1String queryString("SELECT * from MasterKeys where userid=1");
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

QString MasterKeysTable::selectedRowName()
{

    QList<QTableWidgetItem*> items = d->tableView->selectedItems();

    if(items.isEmpty())
        return QString();

    int rowNr = items.first()->row();

    QTableWidgetItem* item = d->tableView->item(rowNr, 1);

    return item->text();
}

void MasterKeysTable::setupUi()
{
    QVBoxLayout* lay = new QVBoxLayout(this);
    d->tableView = new QTableWidget(this);
    d->tableView->setWordWrap(true);
    setTableHeader();
    lay->addWidget(d->tableView);
}

void MasterKeysTable::setTableHeader()
{
    QHeaderView* header = d->tableView->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);

    if(d->headerLabelsList.isEmpty())
    {
        d->headerLabelsList << tr("Key Id")
                            << tr("Key Name")
                            << tr("User Id")
                            << tr("Encrypted Key");
    }
    d->tableView->setColumnCount(d->headerLabelsList.size());
    d->tableView->setHorizontalHeaderLabels(d->headerLabelsList);
}
