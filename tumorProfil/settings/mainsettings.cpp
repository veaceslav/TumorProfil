#include "mainsettings.h"

#include <QWidget>
#include <QDialogButtonBox>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QStackedWidget>
#include <QMessageBox>

#include "databasesettings.h"
#include "encryptionsettings.h"

class MainSettings::Private
{
public:
    Private()
    {
        menuItems       = 0;
        menuContent     = 0;
        buttons         = 0;
        dbSettings      = 0;
        encryptSettings = 0;
    }
    QListWidget* menuItems;
    QStackedWidget* menuContent;
    QDialogButtonBox* buttons;

    DatabaseSettings* dbSettings;
    EncryptionSettings* encryptSettings;

    bool dbOnly;
};
MainSettings::MainSettings(bool dbOnly, QWidget *parent)
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
    setContent(dbOnly);


    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(reject()));

    adjustSize();
}

MainSettings::~MainSettings()
{

}

void MainSettings::accept()
{
    bool scheduleRestart = false;
    if(d->dbSettings != 0)
        d->dbSettings->applySettings();

    if(d->encryptSettings != 0)
        d->encryptSettings->saveSettings(scheduleRestart);

    if(scheduleRestart)
    {
        QMessageBox::information(this, tr("Program will restart"),
                                 tr("One or more settings require the program to restart."));
    }
    QDialog::accept();
}

void MainSettings::slotCurrentPageChanged(int index)
{
    d->menuContent->setCurrentIndex(index);
}

void MainSettings::setContent(bool dbOnly)
{
    connect(d->menuItems, SIGNAL(currentRowChanged(int)), this, SLOT(slotCurrentPageChanged(int)));
    d->dbSettings = new DatabaseSettings(d->menuContent);
    d->menuContent->addWidget(d->dbSettings);
    QListWidgetItem* dbItem = new QListWidgetItem(tr("Database Options"));
    dbItem->setTextAlignment(Qt::AlignCenter);
    d->menuItems->addItem(dbItem);


    if(!dbOnly)
    {
        d->encryptSettings = new EncryptionSettings(d->menuContent);
        d->menuContent->addWidget(d->encryptSettings);
        QListWidgetItem* encryptionItem = new QListWidgetItem(tr("Encryption"));
        encryptionItem->setTextAlignment(Qt::AlignCenter);
        d->menuItems->addItem(encryptionItem);
    }
}

