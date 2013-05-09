/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 20.05.2012
 *
 * Copyright (C) 2012 by Marcel Wiesweg <marcel dot wiesweg at uk-essen dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "analysistableview.h"

// Qt includes

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QDebug>
#include <QFileDialog>
#include <QHeaderView>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QScopedPointer>
#include <QSettings>
#include <QStyledItemDelegate>
#include <QVBoxLayout>

// Local includes

#include "columnselectiondialog.h"
#include "csvfile.h"

class AnalysisDelegate : public QStyledItemDelegate
{
public:

    AnalysisDelegate(QObject* parent = 0) : QStyledItemDelegate(parent)
    {
    }

    virtual QString	displayText(const QVariant& value, const QLocale& locale) const
    {
        if (value.type() == QVariant::Double)
        {
            double d = value.toDouble();
            if (d >= 0 && d <= 1.0)
            {
                return QString::number(100*d, 'f', 2) + " %";
            }
        }
        return QStyledItemDelegate::displayText(value, locale);
    }
};

AnalysisTableView::AnalysisTableView(QWidget *parent) :
    QTableView(parent)
{
}

void AnalysisTableView::keyPressEvent(QKeyEvent *event)
{
    if(event->matches(QKeySequence::Copy) )
    {
      copy();
    }
    else
    {
      QTableView::keyPressEvent(event);
    }
}

void AnalysisTableView::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu(this);
    addContextMenuActions(&menu);
    menu.exec(event->globalPos());
}

void AnalysisTableView::addContextMenuActions(QMenu* menu)
{
    QAction* copyAction = menu->addAction(tr("Kopieren"), this, SLOT(copy()));
    /*QAction* csvAction  = */
    menu->addAction(tr("CSV-Export"), this, SLOT(toCSV()));

    copyAction->setEnabled(selectionModel()->hasSelection());
}

void AnalysisTableView::installDelegateToDisplayPercentages()
{
    setItemDelegate(new AnalysisDelegate(this));
}

void AnalysisTableView::copy()
{
    if (!selectionModel()->hasSelection())
    {
        return;
    }

    QModelIndexList list = selectionModel()->selectedIndexes();
    qSort(list);

    QString copy;
    const bool addVerticalHeader = verticalHeader()->isVisible();

    QModelIndexList::const_iterator it, next;
    // horizontal header
    if (addVerticalHeader)
    {
        copy += "\t\t";
    }
    for (it = list.begin(); it != list.end(); ++it)
    {
        copy += model()->headerData(it->column(), Qt::Horizontal).toString();
        next = it+1;

        if (next == list.end() || it->row() != next->row())
        {
            copy += '\n';
            break;
        }
        else
        {
            copy += '\t';
        }
    }
    copy += '\n';
    // Data
    for (it = list.begin(); it != list.end(); ++it)
    {
        // vertical header
        if (addVerticalHeader && (it == list.begin() || it->row() != (it-1)->row()))
        {
            copy += model()->headerData(it->row(), Qt::Vertical).toString() + "\t\t";
        }

        QVariant value = it->data();
        QString stringValue;
        switch (value.type())
        {
        case QVariant::Double:
            // so that Excel accepts it's a number
            stringValue = QLocale::system().toString(value.toDouble());
            break;
        case QVariant::Int:
        default:
            stringValue = value.toString();
        }

        copy += stringValue;
        next = it+1;

        if (next == list.end() || it->row() != next->row())
        {
            copy += '\n';
        }
        else
        {
            copy += '\t';
        }
    }

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(copy);
}

void AnalysisTableView::toCSV()
{
    QScopedPointer<ColumnSelectionDialog> dialog(new ColumnSelectionDialog(model(), this));

    if (dialog->exec() == QDialog::Rejected)
    {
        return;
    }

    QString csv = toCSV(dialog->columns);

    switch (dialog->result)
    {
    case ColumnSelectionDialog::ToClipboard:
    {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(csv);
        break;
    }
    case ColumnSelectionDialog::Save:
    {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), QString(),
                                                        tr("CSV-Datei (*.csv)"));
        if (fileName.isEmpty())
        {
            break;
        }
        QFile file(fileName);
        file.open(QFile::WriteOnly | QFile::Truncate);
        file.write(csv.toUtf8());
        file.close();
        break;
    }
    default:
        break;
    }

}

QString AnalysisTableView::toCSV(const QList<bool>& columns) const
{
    if (model()->rowCount() == 0)
    {
        return QString();
    }

    CSVFile file;
    QString csv;
    file.writeToString(&csv);

    QList<QVariant> row;

    const bool addVerticalHeader = verticalHeader()->isVisible();

    QItemSelection selection;
    for (int c=0; c<columns.size(); c++)
    {
        if (columns[c])
        {
            selection.select(model()->index(0,c),
                             model()->index(model()->rowCount()-1, c));
        }
    }
    QModelIndexList list = selection.indexes();
    qSort(list);
    QModelIndexList::const_iterator it, next;

    // horizontal header
    if (addVerticalHeader)
    {
        // top left (empty) corner
        row << QVariant();
    }
    for (it = list.begin(); it != list.end(); ++it)
    {
        row << model()->headerData(it->column(), Qt::Horizontal).toString();
        next = it+1;

        if (next == list.end() || it->row() != next->row())
        {
            break;
        }
    }
    file.writeNextLine(row);
    row.clear();

    // Data
    for (it = list.begin(); it != list.end(); ++it)
    {
        // vertical header
        if (addVerticalHeader && (it == list.begin() || it->row() != (it-1)->row()))
        {
            row << model()->headerData(it->row(), Qt::Vertical).toString() + "\t\t";
        }

        QVariant value = it->data();
        QString stringValue;
        switch (value.type())
        {
        case QVariant::Double:
            // so that Excel accepts it's a number
            stringValue = QLocale::system().toString(value.toDouble());
            break;
        case QVariant::Int:
        default:
            stringValue = value.toString();
        }

        row << stringValue;
        next = it+1;

        if (next == list.end() || it->row() != next->row())
        {
            file.writeNextLine(row);
            row.clear();
        }
    }

    return csv;
}
