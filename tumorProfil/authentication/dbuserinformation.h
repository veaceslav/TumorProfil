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

    QString dbUsername();

    QString dbPassword();

    void setUsername(QString username);

    void setPassword(QString password);

    bool loginDbUser();

protected:
    explicit DbUserInformation(QObject *parent = 0);


private:
    class Private;
    Private* const d;
};

#endif // DBUSERINFORMATION_H
