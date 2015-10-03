#include "databaseaccess.h"
#include "databaseconfigelement.h"
#include <QApplication>
#include <QSqlQuery>
#include <QMessageBox>
#include <QSqlError>

#include <QDebug>


QPointer<DatabaseAccess> DatabaseAccess::internalPtr = QPointer<DatabaseAccess>();

class DatabaseAccess::Private
{
public:
    Private()
    {

    }
    DatabaseConfigElement conf;
};
DatabaseAccess* DatabaseAccess::instance()
{
    if(DatabaseAccess::internalPtr.isNull())
        DatabaseAccess::internalPtr = new DatabaseAccess();

    return DatabaseAccess::internalPtr;
}

bool DatabaseAccess::openDb(DatabaseParameters params)
{
    qApp->setOverrideCursor(Qt::WaitCursor);
    QString databaseID(QLatin1String("TumorDbConnectionTest"));

    {

        QSqlDatabase testDatabase     = QSqlDatabase::addDatabase(params.databaseType,
                                                                  databaseID);

        if(!testDatabase.isValid())
        {
            QMessageBox::critical(qApp->activeWindow(), tr("Database connection test"),
                                  tr("Database connection test was not successful. <p>Error was:" +
                                       testDatabase.lastError().text().toLatin1() +  "</p>") );

            return false;
        }

        testDatabase.setHostName(params.hostName);
        testDatabase.setPort(params.port);
        testDatabase.setUserName(params.userName);
        testDatabase.setPassword(params.password);
        testDatabase.setConnectOptions(params.connectOptions);

        qApp->restoreOverrideCursor();

        testDatabase.setDatabaseName(params.databaseName);

        bool result = testDatabase.open();

        if (!result)
        {
            QMessageBox::critical(qApp->activeWindow(), tr("Database connection test"),
                                  tr("Database connection test was not successful. <p>Error was:" +
                                       testDatabase.lastError().text().toLatin1() +  "</p>") );
        }

        QSqlQuery* testQuery = new QSqlQuery(testDatabase);
        testQuery->prepare(QLatin1String("show tables"));

        result = testQuery->exec();

        if (result)
        {
            QMessageBox::information(qApp->activeWindow(), tr("Database connection test"),
                                     tr("Database connection test successful."));
        }
        else
        {
            QMessageBox::critical(qApp->activeWindow(), tr("Database connection test"),
                                  tr("Database connection test was not successful. <p>Error was:" +
                                       testQuery->lastError().text().toLatin1() +  "</p>") );
        }

        testDatabase.close();
    }
    QSqlDatabase::removeDatabase(databaseID);
    return true;
}

bool DatabaseAccess::executeDBAction(QString actionName, QMap<QString, QVariant> bindingMap)
{
    DatabaseAction action = d->conf.sqlStatements.value(actionName);

    if (action.name.isNull())
    {
        qWarning() << "No DB action defined for" << actionName << "! Implementation missing for this database type.";
        return false;
    }

    bool wrapInTransaction = (action.mode == QString("transaction"));

    if (wrapInTransaction)
    {
        beginTransaction();
    }

    foreach(const DatabaseActionElement& actionElement, action.dbActionElements)
    {
//        DatabaseCoreBackend::QueryState result;
        QueryStateEnum result;

        if (actionElement.mode == QString("query"))
        {
            result = executeSql(actionElement.statement, bindingMap);
        }
        else
        {
            result = executeDirectSql(actionElement.statement, bindingMap);
        }

        if (result != DatabaseAccess::NoErrors)
        {
            qDebug() << "Error while executing DBAction ["<<  action.name  <<"] Statement ["<<actionElement.statement<<"]";

            /*
            if (wrapInTransaction && !db.rollback())
            {
                qDebug() << "Error while rollback changes of previous DBAction.";
            }
            */

            break;
        }
    }

    if (wrapInTransaction)
    {
        commitTransaction();
    }
    return true;
}

void DatabaseAccess::setConfigElement(QString type)
{
    d->conf = DatabaseConfigElement::element(type);
}

DatabaseAccess::QueryStateEnum DatabaseAccess::executeSql(QString query, QMap<QString, QVariant> bindValues )
{

    return DatabaseAccess::NoErrors;
}

DatabaseAccess::QueryStateEnum DatabaseAccess::executeDirectSql(QString query, QMap<QString, QVariant> bindValues)
{

    return DatabaseAccess::NoErrors;
}

DatabaseAccess::DatabaseAccess()
    : d(new Private())
{
    setConfigElement(QLatin1String("QMYSQL"));
}

void DatabaseAccess::beginTransaction()
{

}

void DatabaseAccess::commitTransaction()
{

}

