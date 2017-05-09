#ifndef ACCESSMANAGEMENT_H
#define ACCESSMANAGEMENT_H

#include <QStringList>

namespace AccessManagement
{
    enum AccessType{
        None  = 0,
        Read  = 1 << 0,
        Write = 1 << 1,

        ReadWrite = Read | Write
    };

    AccessType accessToPathologyData();

    AccessType accessToDiseaseHistory();

    AccessType check(const QStringList &requiredTables);
};

#endif // ACCESSMANAGEMENT_H
