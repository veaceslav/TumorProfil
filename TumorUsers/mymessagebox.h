#ifndef MYMESSAGEBOX_H
#define MYMESSAGEBOX_H

#include <QString>

class MyMessageBox
{
public:
    MyMessageBox();

    static void showError(QString title, QString errMsg);
    static void showWarning(QString title, QString warningMsg);
};

#endif // MYMESSAGEBOX_H
