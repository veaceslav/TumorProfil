#include "databaseaccess.h"
#include "databaseconfigelement.h"

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

