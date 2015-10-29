#ifndef DATABASEACCESS_H
#define DATABASEACCESS_H

#include <QObject>
#include <QPointer>

#include "databaseparameters.h"

class DatabaseAccess : public QObject
{
    Q_OBJECT
public:

    enum QueryStateEnum
    {
        /**
         * No errors occurred while executing the query.
         */
        NoErrors,

        /**
         * An SQLError has occurred while executing the query.
         */
        SQLError,

        /**
         * An connection error has occurred while executing the query.
         */
        ConnectionError
    };

    static DatabaseAccess* instance();

    bool openDb(DatabaseParameters params);
    bool isOpen();
    bool executeDBAction(QString actionName,
                         QMap<QString, QVariant> bindingMap,
                         QVector<QVector<QVariant> > results);
    void setConfigElement(QString type);

    QStringList tables();
    bool        setSetting(QString setting, QVariant value);
    QString setting(QString value);

    QueryStateEnum executeSql(QString query, QMap<QString, QVariant> bindValues);
    QueryStateEnum executeDirectSql(QString queryString, QMap<QString,
                                    QVariant> bindValues, QVector<QVector<QVariant> > results);

protected:
    DatabaseAccess();
    static QPointer<DatabaseAccess> internalPtr;

private:

    void beginTransaction();
    void commitTransaction();
    class Private;
    Private* d;
};

#endif // DATABASEACCESS_H