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

protected:
    explicit UserQueryUtils();

    static QPointer<UserQueryUtils> internalPtr;


    virtual QueryState executeSql(QString queryString, QMap<QString,
                                  QVariant> bindValues, QVariant& lastId);
    virtual QueryState executeDirectSql(QString queryString, QMap<QString,
                                        QVariant> bindValues, QVector<QVector<QVariant> >& results);

};

#endif // USERQUERYUTILS_H
