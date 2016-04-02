#ifndef MAINSETTINGS_H
#define MAINSETTINGS_H

#include <QDialog>

class MainSettings : public QDialog
{
    Q_OBJECT
public:
    MainSettings(bool dbOnly = false, QWidget* parent = 0);
    ~MainSettings();

public slots:
    virtual void accept();

private slots:
    void slotCurrentPageChanged(int index);
private:

    /**
     * @brief setContent set content of the menu
     * @param dbOnly - set to true if you only want db options, to avoid
     *                 loading settings that might depend on db and the connection is not valid
     */
    void setContent(bool dbOnly);
    class Private;
    Private* d;
};

#endif // MAINSETTINGS_H
