#ifndef ENCRYPTIONSETTINGS_H
#define ENCRYPTIONSETTINGS_H

#include <QWidget>


class EncryptionSettings : public QWidget
{
    Q_OBJECT
public:
    EncryptionSettings(QWidget* parent = 0);

    void loadSettings();
    void saveSettings(bool& schedulerReboot);

    enum EncryptAction { ENCRYPTION = 1, DECRYPTION = 0};
    /**
     * @brief isEncryptionEnabled -check if encryption is enabled in settings
     *        The method is static because we check at the program start-up if
     *        we need to decrypt first
     * @return return true if encryption is enabled.
     */
    static bool isEncryptionEnabled();

private slots:
    /**
     * @brief checkDatabaseEncryptionStatus - when enabling/disabling encryption,
     *        we must check if database is encrypted/decrypted and display the appropriate message
     */
    void checkDatabaseEncryptionStatus(bool value);

    /**
     * @brief slotEncryptDecrypt perform authentication checks and call encrypt/decrypt method
     */
    void slotEncryptDecrypt();

private:
    void setupUi();

    /**
     * @brief encryptDecryptAll perfrom encrypt/decrypt
     * @param action Encrypt or decrypt action
     */
    void encryptDecryptAll(EncryptAction action);

    class Private;
    Private* d;

};

#endif // ENCRYPTIONSETTINGS_H
