#include "databaseaccess.h"
#include "databaseconfigelement.h"
#include <QApplication>
#include <QSqlQuery>
#include <QMessageBox>
#include <QSqlError>

#include <QDebug>

#include "mymessagebox.h"


QPointer<DatabaseAccess> DatabaseAccess::internalPtr = QPointer<DatabaseAccess>();

namespace
{
QString DATABASE_CONNECTION_NAME = QLatin1String("TUMOR_USERS");
}

class DatabaseAccess::Private
{
public:
    Private()
    {
        isActive = false;
    }
    DatabaseConfigElement conf;
    DatabaseParameters params;
    QSqlDatabase database;
    bool isActive;
};

DatabaseAccess* DatabaseAccess::instance()
{
    if(DatabaseAccess::internalPtr.isNull())
        DatabaseAccess::internalPtr = new DatabaseAccess();

    return DatabaseAccess::internalPtr;
}

bool DatabaseAccess::openDb(DatabaseParameters params)
{
    d->isActive = false;
    qApp->setOverrideCursor(Qt::WaitCursor);
    QString databaseID(DATABASE_CONNECTION_NAME);

    {

        d->database = QSqlDatabase::addDatabase(params.databaseType,
                                                                  databaseID);

        if(!d->database.isValid())
        {
            QMessageBox::critical(qApp->activeWindow(), tr("Database connection test"),
                                  tr("Database connection test was not successful. <p>Error was:" +
                                       d->database.lastError().text().toLatin1() +  "</p>") );

            d->isActive = false;
            return false;
        }

        d->database.setHostName(params.hostName);
        d->database.setPort(params.port);
        d->database.setUserName(params.userName);
        d->database.setPassword(params.password);
        d->database.setConnectOptions(params.connectOptions);

        qApp->restoreOverrideCursor();

        d->database.setDatabaseName(params.databaseNameUsers);

        bool result = d->database.open();

        if (!result)
        {
            QMessageBox::critical(qApp->activeWindow(), tr("Database connection test"),
                                  tr("Database connection test was not successful. <p>Error was:" +
                                       d->database.lastError().text().toLatin1() +  "</p>") );
            d->isActive = false;
            d->database.close();
            QSqlDatabase::removeDatabase(databaseID);

        }

        QSqlQuery* testQuery = new QSqlQuery(d->database);
        testQuery->prepare(QLatin1String("show tables"));

        result = testQuery->exec();

        if (result)
        {
            QMessageBox::information(qApp->activeWindow(), tr("Database connection test"),
                                     tr("Database connection test successful."));
            d->isActive = true;
        }
        else
        {
            QMessageBox::critical(qApp->activeWindow(), tr("Database connection test"),
                                  tr("Database connection test was not successful. <p>Error was:" +
                                       testQuery->lastError().text().toLatin1() +  "</p>") );
            d->isActive = false;
            d->database.close();
            QSqlDatabase::removeDatabase(databaseID);
        }
    }
    if(d->isActive)
    {
        d->params = params;
    }
    return d->isActive;
}

bool DatabaseAccess::isOpen()
{
    return d->isActive;
}

bool DatabaseAccess::executeDBAction(QString actionName, QMap<QString, QVariant> bindingMap, QVector<QVector<QVariant> > results)
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
        QueryStateEnum result = DatabaseAccess::NoErrors;

//        qDebug() << "Executing statement " << actionElement.statement << " " << actionElement.mode;
        if (actionElement.mode == QString("query"))
        {
            QVariant id;
            result = executeSql(actionElement.statement, bindingMap, id);
        }
        else
        {
            result = executeDirectSql(actionElement.statement, bindingMap, results);
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

QStringList DatabaseAccess::tables()
{
    QStringList tableList;

    QSqlQuery* query = new QSqlQuery(d->database);
    query->prepare(QLatin1String("show tables"));

    int result = query->exec();

    if(result)
    {
        while(query->next()){
            tableList.append(query->value(0).toString());
        }
    }
    return tableList;
}

bool DatabaseAccess::setSetting(QString setting, QVariant value)
{
    QSqlQuery* query = new QSqlQuery(d->database);
    query->prepare(QLatin1String("INSERT INTO Settings(keyword, value) VALUES(?, ?)"
                                 " ON DUPLICATE KEY UPDATE keyword=VALUES(keyword), value=VALUES(value)"));
    query->addBindValue(setting);
    query->addBindValue(value.toString());

    int result = query->exec();

    qDebug() << "Executing setSettings query, result: " << result;
    if(result)
    {
        return true;
    }
    return false;
}

QString DatabaseAccess::setting(QString value)
{
    QSqlQuery* query = new QSqlQuery(d->database);
    query->prepare(QLatin1String("Select value from Settings where keyword=?"));
    query->bindValue(0, value);

    int result = query->exec();

    if(result)
    {
        while(query->next()){
            return QString(query->value(0).toString());
        }
    }
    return QString();
}

DatabaseAccess::QueryStateEnum DatabaseAccess::executeSql(QString queryString, QMap<QString, QVariant> bindValues, QVariant& lastId)
{

    QSqlQuery* query = new QSqlQuery(d->database);
    query->prepare(queryString);

    for(QMap<QString, QVariant>::iterator it = bindValues.begin(); it !=bindValues.end(); ++it)
    {
        query->bindValue(it.key(),it.value());
    }

    int result = query->exec();


    if(result)
    {
        lastId = query->lastInsertId();
        return DatabaseAccess::NoErrors;
    }
    else
    {
        MyMessageBox::showError(tr("Error Executing Query"),
                                tr("Error while executing query [") + queryString + tr("] Error: ") + query->lastError().text().toLatin1());
        qDebug() << "Error:" << query->lastError().text().toLatin1();
        return DatabaseAccess::SQLError;
    }

}

DatabaseAccess::QueryStateEnum DatabaseAccess::executeDirectSql(QString queryString, QMap<QString, QVariant> bindValues, QVector<QVector<QVariant> >& results)
{

    QSqlQuery* query = new QSqlQuery(d->database);
    query->prepare(queryString);

    for(QMap<QString, QVariant>::iterator it = bindValues.begin(); it !=bindValues.end(); ++it)
    {
        query->bindValue(it.key(),it.value());
    }

    int result = query->exec();



    if(result)
    {
        while(query->next()){
            QVector<QVariant> lst;
            int index = 0;
            QVariant v = query->value(index);
            while(v.isValid())
            {
                lst << v;
                v = query->value(++index);
            }
            results.append(lst);
        }
        return DatabaseAccess::NoErrors;
    }
    else
    {
        qDebug() << "Error:" << query->lastError().text().toLatin1();
        MyMessageBox::showError(tr("Error Executing Query"),
                                tr("Error while executing query [") + queryString + tr("] Error: ") + query->lastError().text().toLatin1());
        return DatabaseAccess::SQLError;
    }
}

DatabaseParameters DatabaseAccess::databaseParams()
{
    return d->params;
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

