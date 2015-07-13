/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 15.06.2015
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

#include "pathologypropertiestableview.h"

#include <QDebug>
#include <QFormLayout>
#include <QHeaderView>
#include <QStyledItemDelegate>

#include "pathologypropertiestablemodel.h"
#include "pathologypropertyinfo.h"
#include "pathologypropertywidget.h"

class PathologyPropertiesTableViewDelegate : public QStyledItemDelegate
{
public:
    PathologyPropertiesTableViewDelegate(QObject* parent = 0)
        : QStyledItemDelegate(parent)
    {
    }

    virtual QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
};

class PathologyPropertiesTableViewEditorWidget : public QWidget
{
public:
    PathologyPropertiesTableViewEditorWidget(PathologyPropertyInfo::Property property, QWidget* parent)
        : QWidget(parent)
    {
        qDebug() << "Creating widget for" << property;
        widget = new PathologyPropertyWidget(property, this);
        QFormLayout* layout = new QFormLayout;
        widget->addToLayout(layout);
        setLayout(layout);
        setAutoFillBackground(true);
    }

    PathologyPropertyWidget *widget;
};


PathologyPropertiesTableView::PathologyPropertiesTableView()
{
    horizontalHeader()->setStretchLastSection(true);
    setItemDelegate(new PathologyPropertiesTableViewDelegate(this));
}

PathologyPropertiesTableModel* PathologyPropertiesTableView::propertiesModel() const
{
    return filterModel()->sourceModel();
}

PathologyPropertiesTableFilterModel* PathologyPropertiesTableView::filterModel() const
{
    return static_cast<PathologyPropertiesTableFilterModel*>(QTableView::model());
}

void PathologyPropertiesTableView::setModels(PathologyPropertiesTableModel* model, PathologyPropertiesTableFilterModel* filterModel)
{
    filterModel->setSourceModel(model);
    QTableView::setModel(filterModel);
}

QWidget* PathologyPropertiesTableViewDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const
{
    PathologyPropertyInfo info = index.data(PathologyPropertiesTableModel::PathologyPropertyInfoRole).value<PathologyPropertyInfo>();
    qDebug() << "createEditor" << info.label << index;
    return new PathologyPropertiesTableViewEditorWidget(info.property, parent);
}

void PathologyPropertiesTableViewDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    PathologyPropertiesTableViewEditorWidget* w = static_cast<PathologyPropertiesTableViewEditorWidget*>(editor);
    model->setData(index, QVariant::fromValue(w->widget->currentProperty()));
}

void PathologyPropertiesTableViewDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    PathologyPropertiesTableViewEditorWidget* w = static_cast<PathologyPropertiesTableViewEditorWidget*>(editor);
    w->widget->setValue(index.data(PathologyPropertiesTableModel::PathologyPropertyRole).value<Property>());
}
