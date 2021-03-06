#include "menubar.h"
#include "settings/mainsettings.h"
#include "authentication/userinformation.h"


class MenuBar::Private
{
public:
    Private()
    {
        loginAction = 0;
    }
    QMenu* fileMenu;
    QMenu* extraMenu;
    QAction* settingsAction;

    QAction* loginAction;
};

MenuBar::MenuBar(QWidget* parent)
    : QMenuBar(parent), d(new Private())
{
    d->fileMenu = new QMenu(tr("File"),this);
    d->fileMenu->addAction(new QAction(tr("Schließen"), this));
    if(UserInformation::instance()->isEncryptionEnabled())
    {
        if(UserInformation::instance()->isLoggedIn())
            d->loginAction = new QAction(tr("Abmelden"), this);
        else
            d->loginAction = new QAction(tr("Anmelden"), this);
        connect(d->loginAction, SIGNAL(triggered(bool)),this,
                SLOT(slotLogIn()));
        d->fileMenu->addAction(d->loginAction);
    }

    d->extraMenu = new QMenu(tr("Extra"),this);
    d->settingsAction = new QAction(tr("Einstellungen"),this);
    d->extraMenu->addAction(d->settingsAction);

    this->addMenu(d->fileMenu);
    this->addMenu(d->extraMenu);

    connect(d->settingsAction, SIGNAL(triggered()), this, SLOT(slotShowSettings()));

    connect(UserInformation::instance(), SIGNAL(signalLoginStateChanged()),
            this, SLOT(updateState()));

}

void MenuBar::slotShowSettings()
{
    MainSettings* settingsDialog = new MainSettings();

    settingsDialog->exec();
}

void MenuBar::slotLogIn()
{
    UserInformation::instance()->toggleLogIn();
    updateState();
}

void MenuBar::updateState()
{
    if(!d->loginAction)
    {
        d->loginAction = new QAction(tr("Abmelden"), this);
    }
    if(UserInformation::instance()->isLoggedIn())
        d->loginAction->setText(tr("Abmelden"));
    else
        d->loginAction->setText(tr("Anmelden"));
}

