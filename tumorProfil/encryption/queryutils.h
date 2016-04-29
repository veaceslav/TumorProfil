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

     QVector<QVector<QVariant> > retrieveMasterKeys(qlonglong userId, QString databaseID);

     QVector<QVector<QVariant> > retrieveUserEntry(const QString& userName, QString databaseID);

     UserDetails retrieveUser(QString name, QString password);

     bool openConnection(DatabaseParameters params, QString databaseID, DatabaseName databaseName);

     bool closeConnection(QString databaseID);

     bool executeDirectSql(QString queryString, QMap<QString,
                                 QVariant> bindValues, QVector<QVector<QVariant> >& results,
                                 QString databaseID);

     bool verifyPassword(const QString& password , const QVector<QVector<QVariant> >& result);

     bool removeAllMasterKeys(int userid, QString databaseID);

     bool updateUserMasterKeys(int userId, QString userPassword, QString userAesFilling,
                                          QMap<QString,QString> userKeys, QString databaseID);
     qlonglong addMasterKey(QString name, qlonglong userid, QString password,
                                  QString aesFilling,  QString databaseID,QString masterKey = QString());


protected:
     explicit TumorQueryUtils();

     static QPointer<TumorQueryUtils> internalPtr;

     virtual QueryState executeSql(QString queryString, QMap<QString,
                                   QVariant> bindValues, QVariant& lastId);
     virtual QueryState executeDirectSql(QString queryString, QMap<QString,
                                         QVariant> bindValues, QVector<QVector<QVariant> >& results);
signals:

public slots:
};

#endif // QUERYUTILS_H
