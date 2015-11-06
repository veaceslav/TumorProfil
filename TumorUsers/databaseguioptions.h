#ifndef DATABASEGUIOPTIONS_H
#define DATABASEGUIOPTIONS_H

#include <QWidget>

class UserWidget;

class DatabaseGuiOptions : public QWidget
{
    Q_OBJECT
public:
    DatabaseGuiOptions(UserWidget *userWidget, QWidget *parent = 0);
    ~DatabaseGuiOptions();

private Q_SLOTS:
    void slotHandleDBTypeIndexChanged();
    void slotSetDatabasePath();
    void slotCheckDatabaseConnection();
    void slotPopulateTable();

private:
    void setupUi();
    void setupParameters();
    class Private;
    Private* d;
};

#endif // DATABASEGUIOPTIONS_H
