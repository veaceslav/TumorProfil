#include "mainsettings.h"

#include <QWidget>
#include <QDialogButtonBox>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QStackedWidget>

#include "databasesettings.h"

class MainSettings::Private
{
public:
    Private()
    {

    }
    QListWidget* menuItems;
    QStackedWidget* menuContent;
    QDialogButtonBox* buttons;
};
MainSettings::MainSettings(QWidget *parent)
    : QDialog(parent), d(new Private())
{
    d->menuItems = new QListWidget(this);
    d->menuContent = new QStackedWidget(this);


    d->buttons = new QDialogButtonBox(QDialogButtonBox::Help | QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QHBoxLayout* mainArea = new QHBoxLayout();
    mainArea->addWidget(d->menuItems, 2);
    mainArea->addWidget(d->menuContent, 6);
    mainLayout->addLayout(mainArea);
    mainLayout->addWidget(d->buttons);
    setContent();


    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(reject()));

    adjustSize();
}

MainSettings::~MainSettings()
{

}

void MainSettings::setContent()
{
    QWidget* databasePage = new DatabaseSettings(d->menuContent);
    d->menuContent->addWidget(databasePage);
    d->menuItems->addItem(new QListWidgetItem(tr("Database Options")));
}

