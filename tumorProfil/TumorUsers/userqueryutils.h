#ifndef USERQUERYUTILS_H
#define USERQUERYUTILS_H

#include "abstractqueryutils.h"

#include <QPointer>

class UserQueryUtils : public AbstractQueryUtils
{
    Q_OBJECT

public:
    static UserQueryUtils* instance();

    ~UserQueryUtils();

    virtual QueryState executeSql(const QString& queryString, QMap<QString,
                                  QVariant> bindValues, QVariant& lastId, const QString& databaseID = QString());
    virtual QueryState executeDirectSql(const QString& queryString, QMap<QString,
                                        QVariant> bindValues, QVector<QVector<QVariant> >& results, const QString& databaseID = QString());

protected:
    explicit UserQueryUtils();

    static QPointer<UserQueryUtils> internalPtr;


};

#endif // USERQUERYUTILS_H
