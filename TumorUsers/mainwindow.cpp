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

    hbox->addWidget(dbGui);
    hbox->addWidget(userW);
    this->setCentralWidget(widget);
}

