#include "menubar.h"
#include "settings/mainsettings.h"


class MenuBar::Private
{
public:
    Private()
    {

    }
    QMenu* fileMenu;
    QMenu* extraMenu;
    QAction* settingsAction;
};

MenuBar::MenuBar(QWidget* parent)
    : QMenuBar(parent), d(new Private())
{
    d->fileMenu = new QMenu(tr("File"),this);
    d->fileMenu->addAction(new QAction(tr("Schließen"), this));

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

