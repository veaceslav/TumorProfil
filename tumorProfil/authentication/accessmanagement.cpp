#include "accessmanagement.h"

#include <QStringList>

#include "userinformation.h"

AccessManagement::AccessManagement(QObject *parent) : QObject(parent)
{

}

AccessManagement::AccessType AccessManagement::accessToPathologyData()
{

    QStringList lst;
    lst << QLatin1String("Patients")
        << QLatin1String("PatientProperties")
        << QLatin1String("Pathologies")
        << QLatin1String("PathologyProperties");

    return check(lst);
}

AccessManagement::AccessType AccessManagement::accessToDiseaseHistory()
{
    QStringList lst;
    lst << QLatin1String("Patients")
        << QLatin1String("Diseases")
        << QLatin1String("DiseaseProperties");

    return check(lst);
}

AccessManagement::AccessType AccessManagement::check(const QStringList& requiredTables)
{
    AccessType type = AccessManagement::WRITE;
    for(QString table : requiredTables)
    {
        int permission = UserInformation::instance()->retrievePermission(table);
        if((int)type > permission){
            type =(AccessType)permission;
        }
    }

    return type;
}
