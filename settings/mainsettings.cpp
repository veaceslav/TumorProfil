#include "mainsettings.h"

#include <QWidget>
#include <QDialogButtonBox>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QStackedWidget>

class MainSettings::Private
{
public:
    Private()
    {

    }
    QTabWidget* mainWidget;
    QListWidget* menuItems;
    QStackedWidget* menuContent;
    QDialogButtonBox* buttons;
};
MainSettings::MainSettings(QWidget *parent)
    : QDialog(parent), d(new Private())
{

    d->mainWidget = new QTabWidget(this);


    d->buttons = new QDialogButtonBox(QDialogButtonBox::Help | QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    mainLayout->addWidget(d->mainWidget);
    mainLayout->addWidget(d->buttons);
    this->setLayout(mainLayout);

    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(reject()));
}

MainSettings::~MainSettings()
{

}

