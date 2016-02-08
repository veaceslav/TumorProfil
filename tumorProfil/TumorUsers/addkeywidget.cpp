#include "addkeywidget.h"

#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QList>
#include <QString>
#include <QPushButton>

class AddKeyWidget::Private
{
public:
    Private()
    {

    }
    QDialogButtonBox* buttons;
    QList<QString> keyNames;
    QComboBox*     box;
};

AddKeyWidget::AddKeyWidget(): d(new Private())
{
    setModal(true);

    d->buttons = new QDialogButtonBox(QDialogButtonBox::Ok   |
                                      QDialogButtonBox::Cancel, this);

    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);

    setHardcodedNames();
    setupUi();

    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(reject()));
}

KeyInfo AddKeyWidget::addKey()
{
    KeyInfo info;

    AddKeyWidget * keyw = new AddKeyWidget();

    int result = keyw->exec();

    if(result == QDialog::Accepted)
        info.name = keyw->currentItem();

    return info;
}

QString AddKeyWidget::currentItem()
{
    return d->box->currentText();
}

void AddKeyWidget::setupUi()
{
    QVBoxLayout* lay = new QVBoxLayout(this);
    QLabel* mainLabel = new QLabel();
    mainLabel->setText(tr("Select a name from the list. Key name should correspond with Patient table field"));
    mainLabel->setWordWrap(true);

    d->box = new QComboBox(this);

    foreach(QString name, d->keyNames)
    {
        d->box->addItem(name, name);
    }

    lay->addWidget(mainLabel);
    lay->addWidget(d->box);
    lay->addWidget(d->buttons);
}

void AddKeyWidget::setHardcodedNames()
{
    d->keyNames << "firstName"
                << "surname"
                << "dateOfBirth";
}

