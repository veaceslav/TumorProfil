#ifndef ACCESSMANAGEMENT_H
#define ACCESSMANAGEMENT_H

#include <QObject>
#include <QStringList>

class AccessManagement : public QObject
{
    Q_OBJECT
public:

    enum AccessType{
        NONE = 0,
        READ = 1,
        WRITE = 2
    };
    explicit AccessManagement(QObject *parent = 0);

    static AccessType accessToPathologyData();

    static AccessType accessToDiseaseHistory();

signals:

public slots:

private:
    static AccessType check(const QStringList &requiredTables);
};

#endif // ACCESSMANAGEMENT_H
