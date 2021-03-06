#include "userqueryutils.h"

#include "databaseaccess.h"

QPointer<UserQueryUtils> UserQueryUtils::internalPtr = QPointer<UserQueryUtils>();

UserQueryUtils::UserQueryUtils()
{

}

AbstractQueryUtils::QueryState UserQueryUtils::executeSql(const QString& queryString, QMap<QString,
                                                          QVariant> bindValues, QVariant &lastId)
{
    return (AbstractQueryUtils::QueryState)DatabaseAccess::instance()->executeSql(queryString,bindValues,lastId);
}

AbstractQueryUtils::QueryState UserQueryUtils::executeDirectSql(const QString& queryString,
                                                                QMap<QString, QVariant> bindValues,
                                                                QVector<QVector<QVariant> > &results)
{
    return (AbstractQueryUtils::QueryState)DatabaseAccess::instance()->executeDirectSql(queryString,bindValues,results );
}

UserQueryUtils* UserQueryUtils::instance()
{
    if(UserQueryUtils::internalPtr.isNull())
        UserQueryUtils::internalPtr = new UserQueryUtils();

    return UserQueryUtils::internalPtr;
}

UserQueryUtils::~UserQueryUtils()
{

}
