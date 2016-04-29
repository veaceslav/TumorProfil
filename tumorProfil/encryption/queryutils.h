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

    enum DatabaseName{
        TUMORPROFIL = 0,
        TUMORUSER = 1
    };


    static TumorQueryUtils* instance();

     QString encpryptMasterKey(QString password, QString filling, QString masterKey);

     QString decryptMasterKey(QString password, QString filling, QString masterHash);

     //QVector<QVector<QVariant> > retrieveMasterKeys(qlonglong userId, QString databaseID);

     QVector<QVector<QVariant> > retrieveUserEntry(const QString& userName, QString databaseID);

     UserDetails retrieveUser(QString name, QString password);

     bool openConnection(DatabaseParameters params, QString databaseID, DatabaseName databaseName);

     bool closeConnection(QString databaseID);

     bool executeDirectSql(QString queryString, QMap<QString,
                                 QVariant> bindValues, QVector<QVector<QVariant> >& results,
                                 QString databaseID);

     bool verifyPassword(const QString& password , const QVector<QVector<QVariant> >& result);



protected:
     explicit TumorQueryUtils();

     static QPointer<TumorQueryUtils> internalPtr;

     virtual QueryState executeSql(QString queryString, QMap<QString,
                                   QVariant> bindValues, QVariant& lastId);
     virtual QueryState executeDirectSql(QString queryString, QMap<QString,
                                         QVariant> bindValues, QVector<QVector<QVariant> >& results);

private:
     class Private;
     Private* const d;
};

#endif // QUERYUTILS_H
