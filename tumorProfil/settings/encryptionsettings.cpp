#include "encryptionsettings.h"
#include "constants.h"

#include <QCheckBox>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QSettings>
#include <QPushButton>
#include <QLabel>
#include <QFormLayout>

#include "patient.h"
#include "databaseaccess.h"
#include "patientdb.h"
#include "encryption/userinformation.h"

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
        initialCheckBoxValue = false;
    }
    QCheckBox* enableEncryption;
    QPushButton* encryptButton;
    QLabel* notificationLabel;
    bool initialCheckBoxValue;
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

    d->initialCheckBoxValue = qs.value(configEnableEncryption, QVariant(false)).toBool();

    if(d->initialCheckBoxValue)
        d->enableEncryption->setCheckState(Qt::Checked);

    checkDatabaseEncryptionStatus(d->initialCheckBoxValue);

}

void EncryptionSettings::saveSettings(bool& scheduleRestart)
{

    QSettings qs(ORGANIZATION, APPLICATION);

    qs.beginGroup(QLatin1String(configGroupEncryption));

    qs.setValue(configEnableEncryption, d->enableEncryption->isChecked());

    qs.sync();

    if(d->initialCheckBoxValue != d->enableEncryption->isChecked())
        scheduleRestart = true;
}

bool EncryptionSettings::isEncryptionEnabled()
{
    QSettings qs(ORGANIZATION, APPLICATION);

    qs.beginGroup(QLatin1String(configGroupEncryption));

    return qs.value(configEnableEncryption, QVariant(false)).toBool();
}

void EncryptionSettings::checkDatabaseEncryptionStatus(bool value)
{
    if(value)
    {
        if((DatabaseAccess().db()->setting(DB_ENCRYPTED) == QLatin1String("0"))
           && (DatabaseAccess().db()->setting(DB_ABOUT_TO_BE_ENCRYPTED) == QLatin1String("0")))
        {
            d->notificationLabel->setText(tr("The database is not encrypted, please encrypt \n"
                                             "the database by pressing encrypt/decrypt button"));
            d->notificationLabel->show();
            d->encryptButton->setEnabled(true);
        }
        else
        {
            d->notificationLabel->clear();
            d->notificationLabel->hide();
            d->encryptButton->setEnabled(false);
        }
    }
    else
    {
        if((DatabaseAccess().db()->setting(DB_ENCRYPTED) == QLatin1String("1"))
           || (DatabaseAccess().db()->setting(DB_ABOUT_TO_BE_ENCRYPTED) == QLatin1String("1")))
        {
            d->notificationLabel->setText(tr("The database is encrypted, please dencrypt"
                                             "the database by pressing encrypt/decrypt button"));
            d->notificationLabel->show();
            d->encryptButton->setEnabled(true);
        }
        else
        {
            d->notificationLabel->clear();
            d->notificationLabel->hide();
            d->encryptButton->setEnabled(false);
        }
    }
}

void EncryptionSettings::setupUi()
{
    QVBoxLayout* const layout  = new QVBoxLayout(this);

    // --------------------------------------------------------

    QGroupBox* dbPathBox = new QGroupBox(tr("Encryption"), this);

    QFormLayout* const form = new QFormLayout(dbPathBox);

    d->enableEncryption = new QCheckBox(tr("Enable Encryption"));
    d->encryptButton    = new QPushButton(tr("Encrypt/Decrypt Database"));
    d->notificationLabel = new QLabel();

    connect(d->enableEncryption, SIGNAL(toggled(bool)),
            this, SLOT(checkDatabaseEncryptionStatus(bool)));

    d->encryptButton->setEnabled(false);

    QPalette sample_palette;
    sample_palette.setColor(QPalette::Window, Qt::yellow);

    d->notificationLabel->setAutoFillBackground(true);
    d->notificationLabel->setPalette(sample_palette);
    d->notificationLabel->hide();



    form->addWidget(d->enableEncryption);
    form->addWidget(d->encryptButton);
    form->addWidget(d->notificationLabel);
    layout->addWidget(dbPathBox);
}

void EncryptionSettings::encryptDecryptAll(EncryptionSettings::EncryptAction action)
{
    if(action == EncryptionSettings::ENCRYPTION)
    {
        UserInformation::instance()->setEncryptionEnabled(false);
    }
    else
    {
        UserInformation::instance()->setEncryptionEnabled(true);
    }

    QList<Patient> patients = DatabaseAccess().db()->findPatients();

    if(action == EncryptionSettings::ENCRYPTION)
    {
        UserInformation::instance()->setEncryptionEnabled(true);
    }
    else
    {
        UserInformation::instance()->setEncryptionEnabled(false);
    }

    for(Patient p : patients)
    {
        DatabaseAccess().db()->updatePatient(p);
    }
}
