#include "menubar.h"
#include "settings/mainsettings.h"
#include "encryption/userinformation.h"


class MenuBar::Private
{
public:
    Private()
    {

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

}

void MenuBar::slotShowSettings()
{
    MainSettings* settingsDialog = new MainSettings();

    settingsDialog->exec();
}

void MenuBar::slotLogIn()
{
    UserInformation::LoginState state =UserInformation::instance()->toggleLogIn();

    if(state == UserInformation::LOGGEDIN)
        d->loginAction->setText(tr("Abmelden"));
    else
        d->loginAction->setText(tr("Anmelden"));
}

