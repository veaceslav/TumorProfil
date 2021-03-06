#ifndef DATABASEGUIOPTIONS_H
#define DATABASEGUIOPTIONS_H

#include <QWidget>

class UserWidget;

class DatabaseGuiOptions : public QWidget
{
    Q_OBJECT
public:
    DatabaseGuiOptions(QWidget *parent = 0);
    ~DatabaseGuiOptions();

signals:
    void signalconnectedToDb();
private Q_SLOTS:
    void slotHandleDBTypeIndexChanged();
    void slotSetDatabasePath();
    void slotCheckDatabaseConnection();

private:
    void setupUi();
    void setupParameters();
    class Private;
    Private* d;
};

#endif // DATABASEGUIOPTIONS_H
