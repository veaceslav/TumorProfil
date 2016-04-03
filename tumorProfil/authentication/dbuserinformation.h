#ifndef DBUSERINFORMATION_H
#define DBUSERINFORMATION_H

#include <QObject>
#include <QPointer>

class DbUserInformation : public QObject
{
    Q_OBJECT

public:
    static DbUserInformation* instance();

    static QPointer<DbUserInformation> internalPtr;

protected:
    explicit DbUserInformation(QObject *parent = 0);

signals:

public slots:
};

#endif // DBUSERINFORMATION_H
