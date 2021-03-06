#ifndef MENUBAR_H
#define MENUBAR_H

#include <QMenuBar>

class MenuBar : public QMenuBar
{
    Q_OBJECT
public:
    MenuBar(QWidget* parent = 0);

private slots:
    void slotShowSettings();
    void slotLogIn();
    void updateState();
private:
    class Private;
    Private* d;
};

#endif // MENUBAR_H
