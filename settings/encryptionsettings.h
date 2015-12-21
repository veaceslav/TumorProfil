#ifndef ENCRYPTIONSETTINGS_H
#define ENCRYPTIONSETTINGS_H

#include <QWidget>


class EncryptionSettings : public QWidget
{
public:
    EncryptionSettings(QWidget* parent = 0);

private:
    void setupUi();

    class Private;
    Private* d;

};

#endif // ENCRYPTIONSETTINGS_H
