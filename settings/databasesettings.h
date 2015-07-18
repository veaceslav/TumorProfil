#ifndef DATABASESETTINGS_H
#define DATABASESETTINGS_H

#include <QWidget>

class DatabaseSettings : public QWidget
{
    Q_OBJECT
public:
    DatabaseSettings(QWidget *parent = 0);

private Q_SLOTS:
    void slotHandleDBTypeIndexChanged(int index);
    void slotSetDatabasePath();

private:

    void setupMainArea();
    void setupSQLiteOptions();
    void setupMySQLOPtions();
    void setDatabaseInputFields(const QString& currentIndexStr);


    class Private;
    Private *d;
};

#endif // DATABASESETTINGS_H
