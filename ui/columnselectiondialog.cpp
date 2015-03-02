#include "columnselectiondialog.h"

#include <QAbstractItemModel>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QList>
#include <QSettings>
#include <QVBoxLayout>

ColumnSelectionDialog::ColumnSelectionDialog(QAbstractItemModel* model, QWidget*)
{
    QVBoxLayout *layout = new QVBoxLayout;
    QLabel *label = new QLabel(tr("Bitte Spalten für CSV-Export auswählen:"));
    layout->addWidget(label);

    QSettings settings;
    const QString settingsKey = "AnalysisTableView/CSVExport/";

    for (int col=0; col<model->columnCount(); col++)
    {
        QString title = model->headerData(col, Qt::Horizontal).toString();
        horizontalTitles << title;
        settingsKeys << settingsKey + title;
    }

    for (int i=0; i<horizontalTitles.size(); i++)
    {
        QCheckBox* box = new QCheckBox(horizontalTitles[i]);
        boxes << box;
        box->setChecked(settings.value(settingsKeys[i], true).toBool());
        layout->addWidget(box);
    }

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                     QDialogButtonBox::Cancel |
                                     QDialogButtonBox::Save);
    layout->addWidget(buttonBox);
    connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(slotClicked(QAbstractButton*)));
    setLayout(layout);
}

void ColumnSelectionDialog::slotClicked(QAbstractButton* button)
{
    switch (buttonBox->standardButton(button))
    {
    case QDialogButtonBox::Ok:
        result = ToClipboard;
        accept();
        break;
    case QDialogButtonBox::Save:
        result = Save;
        accept();
        break;
    default:
    case QDialogButtonBox::Cancel:
        result = Cancel;
        reject();
        break;
    }

    columns.clear();
    if (result != Cancel)
    {
        QVariantList states;
        QSettings settings;
        for (int i=0; i<horizontalTitles.size(); i++)
        {
            bool b = boxes[i]->isChecked();
            columns << b;
            settings.setValue(settingsKeys[i], b);
        }
    }
}

