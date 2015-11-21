#include "mymessagebox.h"

#include <QMessageBox>
#include <QApplication>

MyMessageBox::MyMessageBox()
{

}

void MyMessageBox::showError(QString title, QString errMsg)
{
    QMessageBox::critical(qApp->activeWindow(), title,
                          errMsg );
}

void MyMessageBox::showWarning(QString title, QString warningMsg)
{
    QMessageBox::warning(qApp->activeWindow(), title,
                         warningMsg );
}

void MyMessageBox::showInfo(QString title, QString infoMsg)
{
    QMessageBox::information(qApp->activeWindow(), title,
                         infoMsg );
}

