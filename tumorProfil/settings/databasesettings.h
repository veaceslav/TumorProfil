#ifndef DATABASESETTINGS_H
#define DATABASESETTINGS_H

#include <QWidget>

#include "databaseparameters.h"

class DatabaseSettings : public QWidget
{
    Q_OBJECT
public:
    DatabaseSettings(QWidget *parent = 0);

    QString currentDatabaseType();

    void loadSettings();
    void applySettings();

    /**
     * @brief checkDatabaseConnection - check if program can connect to MySQL database
     * @param parameters - database parameters such as host, port, username etc...
     * @return
     */
    static bool checkDatabaseConnection(DatabaseParameters parameters);

private Q_SLOTS:
    void slotHandleDBTypeIndexChanged(int index);
    void slotSetDatabasePath();
    void slotCheckDatabaseConnection();


private:
    void setupMainArea();
    void setDatabaseInputFields(const QString& currentIndexStr);
    DatabaseParameters getDatabaseParameters();


    class Private;
    Private *d;
};

#endif // DATABASESETTINGS_H
