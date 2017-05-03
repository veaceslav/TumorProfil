#ifndef QUERYUTILS_H
#define QUERYUTILS_H
#include <QObject>

#include <QMap>
#include "databaseparameters.h"
#include "TumorUsers/abstractqueryutils.h"


class QSqlDatabase;


class TumorQueryUtils : public AbstractQueryUtils
{
    Q_OBJECT
public:
    enum UserType{
        ADMIN  = 0,
        USER = 1
    };

    static bool open(const DatabaseParameters& params);

    static TumorQueryUtils* instance();

    QString encpryptMasterKey(const QString& password, const QString& salt, const QString& masterKey);

    QString decryptMasterKey(const QString& password, const QString& salt, const QString& masterHash);

    //QVector<QVector<QVariant> > retrieveMasterKeys(qlonglong userId, const QString& databaseID);

    QVector<QVector<QVariant> > retrieveUserEntry(const QString& userName);

    UserDetails retrieveUser(const QString& name, const QString& password);

    bool verifyPassword(const QString& password , const QVector<QVector<QVariant> >& result);

    virtual QueryState executeSql(const QString& queryString, QMap<QString,
                                  QVariant> bindValues, QVariant& lastId);
    virtual QueryState executeDirectSql(const QString& queryString, QMap<QString,
                                        QVariant> bindValues, QVector<QVector<QVariant> >& results);
protected:
    explicit TumorQueryUtils();

    static QPointer<TumorQueryUtils> internalPtr;

private:
    class Private;
    Private* const d;
};

#endif // QUERYUTILS_H
