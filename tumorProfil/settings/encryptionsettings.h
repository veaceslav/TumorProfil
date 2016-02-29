#ifndef ENCRYPTIONSETTINGS_H
#define ENCRYPTIONSETTINGS_H

#include <QWidget>


class EncryptionSettings : public QWidget
{
public:
    EncryptionSettings(QWidget* parent = 0);

    void loadSettings();
    void saveSettings();

    enum EncryptAction { ENCRYPTION = 1, DECRYPTION = 2};
    /**
     * @brief isEncryptionEnabled -check if encryption is enabled in settings
     *        The method is static because we check at the program start-up if
     *        we need to decrypt first
     * @return return true if encryption is enabled.
     */
    static bool isEncryptionEnabled();
private:
    void setupUi();

    void encryptDecryptAll(EncryptAction action);

    class Private;
    Private* d;

};

#endif // ENCRYPTIONSETTINGS_H
