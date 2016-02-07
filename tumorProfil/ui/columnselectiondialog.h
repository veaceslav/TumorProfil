#ifndef COLUMNSELECTIONDIALOG_H
#define COLUMNSELECTIONDIALOG_H

#include <QDialog>

class QAbstractItemModel;
class QAbstractButton;
class QCheckBox;
class QDialogButtonBox;

class ColumnSelectionDialog : public QDialog
{
    Q_OBJECT

public:

    ColumnSelectionDialog(QAbstractItemModel* model, QWidget* parent = 0);

    enum Result
    {
        ToClipboard,
        Save,
        Cancel
    };

    QList<bool>  columns;
    Result       result;

protected slots:

    void slotClicked(QAbstractButton* button);

protected:

    QList<QCheckBox*> boxes;
    QStringList horizontalTitles;
    QStringList settingsKeys;
    QDialogButtonBox* buttonBox;
};

#endif // COLUMNSELECTIONDIALOG_H
