#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private Q_SLOTS:
    void slotListUsers();
    bool slotAddUser();
    void slotAddEncryptionKey();
    void slotEditUser();
    void slotDeleteUser();

private:
    void setupUi();
    void setupToolBar();

    class Private;
    Private* d;
};

#endif // MAINWINDOW_H
