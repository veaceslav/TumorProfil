#include "encryptionsettings.h"
#include "constants.h"

#include <QCheckBox>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QSettings>
#include <QPushButton>

namespace
{
    const char* configGroupEncryption = "Encryption";
    const char* configEnableEncryption = "Enable_Encryption";
}
class EncryptionSettings::Private
{

public:
    Private()
    {

    }
    QCheckBox* enableEncryption;
    QPushButton* encryptButton;
};
EncryptionSettings::EncryptionSettings(QWidget *parent)
    : QWidget(parent), d(new Private())
{
    setupUi();
    loadSettings();
}

void EncryptionSettings::loadSettings()
{
    QSettings qs(ORGANIZATION, APPLICATION);

    qs.beginGroup(QLatin1String(configGroupEncryption));

    bool val = qs.value(configEnableEncryption, QVariant(false)).toBool();

    if(val)
        d->enableEncryption->setCheckState(Qt::Checked);

}

void EncryptionSettings::saveSettings()
{

    QSettings qs(ORGANIZATION, APPLICATION);

    qs.beginGroup(QLatin1String(configGroupEncryption));

    qs.setValue(configEnableEncryption, d->enableEncryption->isChecked());

    qs.sync();
}

bool EncryptionSettings::isEncryptionEnabled()
{
    QSettings qs(ORGANIZATION, APPLICATION);

    qs.beginGroup(QLatin1String(configGroupEncryption));

    return qs.value(configEnableEncryption, QVariant(false)).toBool();
}

void EncryptionSettings::setupUi()
{
    QVBoxLayout* const layout  = new QVBoxLayout(this);

    // --------------------------------------------------------

    QGroupBox* dbPathBox = new QGroupBox(tr("Encryption"), this);
    QVBoxLayout* const vlay    = new QVBoxLayout(dbPathBox);

    d->enableEncryption = new QCheckBox(tr("Enable Encryption"));
    d->encryptButton    = new QPushButton(tr("Encrypt/Decrypt Database"));
    vlay->addWidget(d->enableEncryption);
    vlay->addWidget(d->encryptButton);
    layout->addWidget(dbPathBox);

}

void EncryptionSettings::encryptDecryptAll(EncryptionSettings::EncryptAction action)
{
    //QList<Patients> patients = DatabaseAccess().db()->findPatients();
}
