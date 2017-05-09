#include "accessmanagement.h"

#include <QStringList>

#include "userinformation.h"
#include "TumorUsers/abstractqueryutils.h"

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
    AccessType type = ReadWrite;
    for(const QString& table : requiredTables)
    {
        AccessType tableRights;
        switch (UserInformation::instance()->retrievePermission(table))
        {
        case AbstractQueryUtils::PERMISSION_NONE:
            tableRights = None;
            break;
        case AbstractQueryUtils::PERMISSION_READ:
            tableRights = Read;
            break;
        case AbstractQueryUtils::PERMISSION_READWRITE:
            tableRights = ReadWrite;
        }

        type = AccessType(type & tableRights);
    }

    return type;
}
