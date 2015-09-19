#include "mainwindow.h"

#include <QLineEdit>
#include <QString>
#include <QGroupBox>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QApplication>
#include <QFileDialog>
#include <QSqlDatabase>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QDesktopWidget>

#include "databaseconfigelement.h"
#include "databaseguioptions.h"
#include "userwidget.h"


class MainWindow::Private
{
public:
    Private()
    {

    }

};
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), d(new Private())
{
    setupUi();
    resize(QDesktopWidget().availableGeometry(this).size() * 0.7);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUi()
{
    QWidget* widget = new QWidget(this);
    QHBoxLayout* hbox = new QHBoxLayout(widget);
    DatabaseGuiOptions* dbGui = new DatabaseGuiOptions(widget);
    UserWidget* userW         = new UserWidget(widget);

    hbox->addWidget(dbGui,2);
    hbox->addWidget(userW,6);
    this->setCentralWidget(widget);
}

