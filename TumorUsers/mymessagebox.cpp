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

void MyMessageBox::showWarning(QString title, QString errMsg)
{
    QMessageBox::warning(qApp->activeWindow(), title,
                          errMsg );
}

