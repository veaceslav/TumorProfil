/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 28.02.2013
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

#include "historywindow.h"

// Qt includes

#include <QAction>
#include <QDateEdit>
#include <QLineEdit>
#include <QDebug>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QScrollArea>
#include <QToolBar>
#include <QToolButton>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QTreeView>
#include <QVBoxLayout>

// Local includes

#include "historypatientlistview.h"
#include "diseasehistorymodel.h"
#include "historyelementeditwidget.h"
#include "patientdisplay.h"
#include "patientmanager.h"
#include "visualhistorywidget.h"


class HistoryElementSortModel : public QSortFilterProxyModel
{
public:

    HistoryElementSortModel(QObject* parent)
        : QSortFilterProxyModel(parent)
    {
        setDynamicSortFilter(true);
    }

    virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const
    {
        HistoryElement* l = DiseaseHistoryModel::retrieveElement(left);
        HistoryElement* r = DiseaseHistoryModel::retrieveElement(right);
        if (!l || !r)
        {
            return false;
        }
        if (l->date.isValid() && r->date.isValid())
        {
            if (l->date == r->date)
            {
                if (l->is<Therapy>() || r->is<Therapy>())
                {
                    if (l->is<Therapy>() != r->is<Therapy>())
                    {
                        // only one is Therapy
                        return !l->is<Therapy>();
                    }
                    else
                    {
                        // both are therapies
                        return l->as<Therapy>()->end < r->as<Therapy>()->end;
                    }
                }
            }
            return l->date < r->date;
        }
        else
        {
            // only one is valid
            if (l->date.isValid() || r->date.isValid())
            {
                // valid date is "less than" invalid
                return l->date.isValid();
            }
            return false;
        }
    }
};

class HistoryWindow::Private
{
public:
    Private()
        : viewSplitter(0),
          patientView(0),
          historyModel(0),
          heading(0),
          clearButton(0),
          mainPanelLayout(0),
          editWidgetLayout(0),
          visualHistoryWidget(0),
          patientDisplay(0),
          sortModel(0),
          historyView(0),
          addBar(0),
          initialDiagnosisEdit(0),
          tnmEdit(0),
          currentElement(0),
          currentWidget(0)
    {
    }

    QSplitter              *viewSplitter;
    HistoryPatientListView *patientView;
    DiseaseHistoryModel    *historyModel;
    QLabel                 *heading;
    QPushButton            *clearButton;
    QVBoxLayout            *mainPanelLayout;
    QVBoxLayout            *editWidgetLayout;
    VisualHistoryWidget    *visualHistoryWidget;
    PatientDisplay         *patientDisplay;
    HistoryElementSortModel*sortModel;
    QTreeView              *historyView;
    QToolBar               *addBar;
    QDateEdit              *initialDiagnosisEdit;
    QLineEdit              *tnmEdit;

    Patient::Ptr              currentPatient;
    HistoryElement           *currentElement;
    HistoryElementEditWidget *currentWidget;
};

HistoryWindow::HistoryWindow(QWidget *parent)
    : FilterMainWindow(parent),
      d(new Private)
{
    setWindowTitle(tr("Krankheitsverlauf"));
    setWindowIcon(QIcon::fromTheme("calendar"));
    setupToolbar();
    setupView();
    setCurrentPatient(Patient::Ptr());
    setCurrentElement(0);
}

HistoryWindow::~HistoryWindow()
{
    setCurrentPatient(Patient::Ptr());
    delete d;
}

void HistoryWindow::closeEvent(QCloseEvent *e)
{
    applyData();
    FilterMainWindow::closeEvent(e);
}

Patient::Ptr HistoryWindow::currentPatient() const
{
    return d->currentPatient;
}

HistoryElement* HistoryWindow::currentElement() const
{
    return d->currentElement;
}

DiseaseHistory HistoryWindow::currentHistory() const
{
    if (!d->currentPatient)
    {
        return DiseaseHistory();
    }
    return d->historyModel->history();
}

void HistoryWindow::applyData()
{
    if (d->currentPatient)
    {
        if (d->currentWidget)
        {
            d->currentWidget->applyToElement();
        }
        Disease& disease = d->currentPatient->firstDisease();
        DiseaseHistory history = d->historyModel->history();
        history.sort();
        disease.setHistory(history);
        //qDebug() << "History of patient" << d->currentPatient->surname;
        //qDebug() << d->currentPatient->firstDisease().history().toXml();
        disease.initialDiagnosis = d->initialDiagnosisEdit->date();
        disease.initialTNM.setTNM(d->tnmEdit->text());
        PatientManager::instance()->updateData(d->currentPatient,
                                               PatientManager::ChangedDiseaseHistory |
                                               PatientManager::ChangedDiseaseMetadata);
        PatientManager::instance()->historySecurityCopy(d->currentPatient, "history", d->currentPatient->firstDisease().history().toXml());

    }
}

void HistoryWindow::setCurrentPatient(Patient::Ptr p)
{
    applyData();
    setCurrentElement(0);
    d->currentPatient = p;
    d->initialDiagnosisEdit->setDate(QDate::currentDate());

    if (d->currentPatient)
    {
        const Disease& disease = d->currentPatient->firstDisease();
        DiseaseHistory history = disease.history();
        d->historyModel->setHistory(history);
        d->initialDiagnosisEdit->setDate(disease.initialDiagnosis);
        d->tnmEdit->setText(disease.initialTNM.toText());
        d->visualHistoryWidget->setHistory(history);
    }
    d->patientDisplay->setPatient(d->currentPatient);
    d->addBar->setEnabled(d->currentPatient);
}

void HistoryWindow::historyElementActivated(const QModelIndex& index)
{
    HistoryElement* elem = DiseaseHistoryModel::retrieveElement(index);
    while (elem->parent())
    {
        elem = elem->parent();
    }
    qDebug() << "history element activated" << elem;
    setCurrentElement(elem);
}

void HistoryWindow::setCurrentElement(HistoryElement* e)
{
    if (d->currentElement && d->currentWidget)
    {
         d->currentWidget->applyToElement();
         delete d->currentWidget;
         d->currentWidget = 0;
    }

    if (e)
    {
        d->currentElement = e;
        d->currentWidget = HistoryElementEditWidget::create(e);
        if (d->currentWidget)
        {
            d->editWidgetLayout->addWidget(d->currentWidget);
            connect(d->currentWidget, SIGNAL(changed()), this, SLOT(currentElementChanged()));
            if (d->currentElement->is<Therapy>())
            {
                connect(d->currentWidget, SIGNAL(addTherapyElement(TherapyElement*)),
                        this, SLOT(currentElementAddTherapyElement(TherapyElement*)));
                connect(d->currentWidget, SIGNAL(therapyElementChanged(TherapyElement*)),
                        this, SLOT(currentElementTherapyElementChanged(TherapyElement*)));
                connect(d->currentWidget, SIGNAL(therapyElementRemove(TherapyElement*)),
                        this, SLOT(currentElementTherapyElementRemove(TherapyElement*)));
            }
            d->currentWidget->setFocus();
        }
    }
    d->clearButton->setEnabled(d->currentElement);
    d->heading->setText(d->currentWidget ? d->currentWidget->heading() : QString());
    if (d->currentElement)
    {
        QModelIndex index = d->historyModel->index(d->currentElement);
        d->historyView->expand(index);
        d->historyView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
    }
}

void HistoryWindow::clearCurrentElement()
{
    if (d->currentElement)
    {
        d->historyModel->takeElement(d->currentElement);
        delete d->currentElement;
        d->currentElement = 0;
        delete d->currentWidget;
        d->currentWidget = 0;
        setCurrentElement(0);
    }
}

static void addToolbarMenuAction(QMenu* menu, const QIcon& icon, const QString& text, const QVariant& userData)
{
    QAction* action = menu->addAction(icon, text);
    action->setData(userData);
}

void HistoryWindow::setupView()
{
    d->viewSplitter = new QSplitter(Qt::Horizontal, this);

    // Patient list view
    d->patientView = new HistoryPatientListView;
    d->patientView->setAdapter(adapter());
    connect(d->patientView, SIGNAL(activated(Patient::Ptr)), this, SLOT(setCurrentPatient(Patient::Ptr)));
    connect(d->patientView, SIGNAL(activated(Patient::Ptr)), this, SIGNAL(activated(Patient::Ptr)));

    // History Tree View
    QWidget* secondWidget = new QWidget;
    QVBoxLayout* secondWidgetLayout = new QVBoxLayout;
    d->historyModel = new DiseaseHistoryModel(this);
    d->historyView  = new QTreeView;
    d->sortModel    = new HistoryElementSortModel(this);
    d->sortModel->setSourceModel(d->historyModel);
    d->historyView->setModel(d->sortModel);
    d->sortModel->sort(0);
    connect(d->historyView, SIGNAL(activated(QModelIndex)), this, SLOT(historyElementActivated(QModelIndex)));
    secondWidgetLayout->addWidget(d->historyView, 1);
    secondWidget->setLayout(secondWidgetLayout);

    // Main (editing) panel
    QWidget* mainPanel = new QWidget;
    d->mainPanelLayout = new QVBoxLayout;
    d->mainPanelLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    QScrollArea* visualHistoryWidgetScrollArea = new QScrollArea;
    d->visualHistoryWidget = new VisualHistoryWidget;
    visualHistoryWidgetScrollArea->setWidget(d->visualHistoryWidget);
    visualHistoryWidgetScrollArea->setWidgetResizable(true);
    visualHistoryWidgetScrollArea->setFrameStyle(QFrame::NoFrame);
    d->mainPanelLayout->addWidget(visualHistoryWidgetScrollArea);

    d->patientDisplay = new PatientDisplay;
    d->patientDisplay->setShowGender(false);
    d->mainPanelLayout->addWidget(d->patientDisplay, 1);

    QFormLayout* idAndTnmLayout = new QFormLayout;
    d->initialDiagnosisEdit = new QDateEdit;
    idAndTnmLayout->addRow(tr("Erstdiagnose:"), d->initialDiagnosisEdit);
    d->tnmEdit = new QLineEdit;
    idAndTnmLayout->addRow(tr("TNM (initial):"), d->tnmEdit);
    d->mainPanelLayout->addLayout(idAndTnmLayout);

    // Tool bar
    d->addBar = new QToolBar;
    //d->addBar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    //d->addBar->setOrientation(Qt::Vertical);
    d->addBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->addBar->addAction(QIcon::fromTheme("pill"), tr("Chemo"), this, SLOT(addChemotherapy()));
    QAction* therapyAction = d->addBar->addAction(QIcon::fromTheme("basket"), tr("Therapie..."));
    d->addBar->addAction(QIcon::fromTheme("heart_add"), tr("Befund"), this, SLOT(addFinding()));
    d->addBar->addAction(QIcon::fromTheme("chart_bar"), tr("Zustand"), this, SLOT(addDiseaseState()));

    QToolButton* therapyButton = static_cast<QToolButton*>(d->addBar->widgetForAction(therapyAction));
    therapyButton->setPopupMode(QToolButton::InstantPopup);
    QMenu* therapyMenu = new QMenu;
    addToolbarMenuAction(therapyMenu, QIcon::fromTheme("transmit"), tr("Radiotherapie"), Therapy::RTx);
    addToolbarMenuAction(therapyMenu, QIcon::fromTheme("transmit_add"), tr("Radiochemotherapie"), Therapy::RCTx);
    addToolbarMenuAction(therapyMenu, QIcon::fromTheme("pencil"), tr("Operation"), Therapy::Surgery);
    addToolbarMenuAction(therapyMenu, QIcon::fromTheme("wrench"), tr("Intervention"), Therapy::Intervention);
    connect(therapyMenu, SIGNAL(triggered(QAction*)), this, SLOT(addTherapy(QAction*)));
    therapyButton->setMenu(therapyMenu);
    d->mainPanelLayout->addWidget(d->addBar);

    QHBoxLayout* headingLayout = new QHBoxLayout;
    d->heading = new QLabel;
    QFont f = font();
    f.setBold(true);
    f.setPixelSize(20);
    d->heading->setFont(f);
    d->clearButton = new QPushButton(QIcon::fromTheme("delete"), tr("Löschen"));
    connect(d->clearButton, SIGNAL(clicked()), this, SLOT(clearCurrentElement()));
    headingLayout->addWidget(d->heading);
    headingLayout->addStretch(1);
    headingLayout->addWidget(d->clearButton);
    d->mainPanelLayout->addLayout(headingLayout);

    d->editWidgetLayout = new QVBoxLayout;
    d->mainPanelLayout->addLayout(d->editWidgetLayout);
    d->mainPanelLayout->addStretch(1);

    mainPanel->setLayout(d->mainPanelLayout);
    QScrollArea* scrollArea = new QScrollArea;
    scrollArea->setWidget(mainPanel);

    d->viewSplitter->addWidget(d->patientView);
    d->viewSplitter->addWidget(secondWidget);
    d->viewSplitter->addWidget(scrollArea);
    //d->viewSplitter->addWidget(mainPanel);
    setCentralWidget(d->viewSplitter);

    d->viewSplitter->setSizes(QList<int>() << 250 << 250 << 500);
}

void HistoryWindow::addChemotherapy()
{
    addTherapy(Therapy::CTx);
}

void HistoryWindow::addFinding()
{
    QDate date = dateForNewElement();
    Finding* f = new Finding;
    f->date = date;
    d->historyModel->addElement(f);
    setCurrentElement(f);
}

void HistoryWindow::addDiseaseState()
{
    QDate date = dateForNewElement();
    DiseaseState* s = new DiseaseState;
    s->date = date;
    d->historyModel->addElement(s);
    setCurrentElement(s);
}

void HistoryWindow::addTherapy(QAction* action)
{
    addTherapy((Therapy::Type)action->data().toInt());
}

QDate HistoryWindow::dateForNewElement() const
{
    QDate date;
    if (d->currentElement)
    {
        date = d->currentElement->date;
        if (d->currentElement->is<Therapy>())
        {
            QDate end = d->currentElement->as<Therapy>()->end;
            if (end.isValid())
            {
                date = end;
            }
        }
    }
    if (!date.isValid())
    {
        date = currentHistory().latestDate();
    }
    if (!date.isValid())
    {
        date = QDate::currentDate();
    }
    return date;
}

void HistoryWindow::addTherapy(Therapy::Type type)
{
    QDate date = dateForNewElement();

    Therapy* t = new Therapy;
    t->type = type;
    t->date = date;

    TherapyElement* elem = 0;

    switch (type)
    {
    case Therapy::CTx:
        elem = new Chemotherapy;
        break;
    case Therapy::RTx:
        elem = new Radiotherapy;
        break;
    case Therapy::RCTx:
        elem = new Radiotherapy;
        break;
    case Therapy::Surgery:
    case Therapy::Intervention:
        break;
    }

    if (elem)
    {
        t->elements << elem;
    }
    d->historyModel->addElement(t);
    setCurrentElement(t);
}

void HistoryWindow::currentElementChanged()
{
    qDebug() << "Current element changed";
    if (!d->currentWidget || !d->currentElement)
    {
        return;
    }
    d->currentWidget->applyToElement();
    d->historyModel->elementChanged(d->currentElement);
}

void HistoryWindow::currentElementAddTherapyElement(TherapyElement* te)
{
    d->historyModel->addElement(d->currentElement, te);
}

void HistoryWindow::currentElementTherapyElementChanged(TherapyElement* te)
{
    qDebug() << "Current therapy element changed";
    d->historyModel->elementChanged(te);
}

void HistoryWindow::currentElementTherapyElementRemove(TherapyElement* te)
{
    TherapyEditWidget* tew = qobject_cast<TherapyEditWidget*>(d->currentWidget);
    if (!tew)
    {
        qDebug() << "Bug! TEW is not a TEW";
        return;
    }
    d->historyModel->takeElement(te);
    tew->removeElementUI(te);
    delete te;
}
