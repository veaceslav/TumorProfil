#include "dbuserinformation.h"


QPointer<DbUserInformation> DbUserInformation::internalPtr = QPointer<DbUserInformation>();

DbUserInformation::DbUserInformation(QObject *parent)
    : QObject(parent)
{

}

DbUserInformation *DbUserInformation::instance()
{
    if(DbUserInformation::internalPtr.isNull())
        DbUserInformation::internalPtr = new DbUserInformation();

    return DbUserInformation::internalPtr;
}
