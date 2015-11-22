#ifndef MASTERKEYSTABLE_H
#define MASTERKEYSTABLE_H

#include <QWidget>
#include <QObject>

class MasterKeysTable : public QWidget
{
public:
    MasterKeysTable(QWidget* parent = 0);
    ~MasterKeysTable();

    void populateTable();
    int selectedRowId();
signals:

public slots:

private:
    void setupUi();
    void setTableHeader();
    class Private;
    Private* d;
};


#endif // MASTERKEYSTABLE_H
