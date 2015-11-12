#ifndef USERWIDGET_H
#define USERWIDGET_H

#include <QWidget>

class UserWidget : public QWidget
{
public:
    UserWidget(QWidget* parent = 0);
    ~UserWidget();

    void populateTable();
    void addRow(qlonglong id);
signals:

public slots:

private:
    void setupUi();
    void setTableHeader();
    class Private;
    Private* d;
};

#endif // USERWIDGET_H
