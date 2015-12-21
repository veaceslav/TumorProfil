#include "encryptionsettings.h"

#include <QCheckBox>
#include <QVBoxLayout>
#include <QGroupBox>
class EncryptionSettings::Private
{
public:
    Private()
    {

    }
    QCheckBox* enableEncryption;
};
EncryptionSettings::EncryptionSettings(QWidget *parent)
    : QWidget(parent), d(new Private())
{
    setupUi();
}

void EncryptionSettings::setupUi()
{
    QVBoxLayout* const layout  = new QVBoxLayout(this);

    // --------------------------------------------------------

    QGroupBox* dbPathBox = new QGroupBox(tr("Encryption"), this);
    QVBoxLayout* const vlay    = new QVBoxLayout(dbPathBox);

    d->enableEncryption = new QCheckBox(tr("Enable Encryption"));
    vlay->addWidget(d->enableEncryption);
    layout->addWidget(dbPathBox);

}
