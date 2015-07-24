#ifndef MAINSETTINGS_H
#define MAINSETTINGS_H

#include <QDialog>

class MainSettings : public QDialog
{
    Q_OBJECT
public:
    MainSettings(QWidget* parent = 0);
    ~MainSettings();

public slots:
    virtual void accept();
private:

    void setContent();
    class Private;
    Private* d;
};

#endif // MAINSETTINGS_H
