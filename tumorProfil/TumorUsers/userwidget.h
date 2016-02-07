#ifndef USERWIDGET_H
#define USERWIDGET_H

#include <QWidget>

class UserWidget : public QWidget
{
public:

    enum TableColumns
    {
        USERID_COLUMN = 0,
        USERNAME_COLUMN = 1,
        USERGROUP_COLUMN = 2
    };

    UserWidget(QWidget* parent = 0);
    ~UserWidget();

    void populateTable();
    void addRow(qlonglong id);
    int selectedRowId();
    QList<QVariant> selectedRowData();
signals:

public slots:

private:
    void setupUi();
    void setTableHeader();
    class Private;
    Private* d;
};

#endif // USERWIDGET_H
