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
#include <QCheckBox>
#include <QDateEdit>
#include <QLineEdit>
#include <QDebug>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QScrollArea>
#include <QToolBar>
#include <QToolButton>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QListWidget>
#include <QTreeView>
#include <QVBoxLayout>

// Local includes

#include "historypatientlistview.h"
#include "patientpropertymodelviewadapter.h"
#include "diseasehistorymodel.h"
#include "historyelementeditwidget.h"
#include "history/historyiterator.h"
#include "patientdisplay.h"
#include "patientmanager.h"
#include "patientpropertymodel.h"
#include "visualhistorywidget.h"

class HistoryWindowProofReadOutput : public HistoryProofreader
{
public:
    HistoryWindowProofReadOutput(QListWidget* lw) : list(lw)
    {
    }

    void problem(const HistoryElement *element, const QString &problem)
    {
        QListWidgetItem* item = new QListWidgetItem(problem);
        item->setData(Qt::UserRole, QVariant::fromValue<HistoryElement*>((HistoryElement*)element));
        list->addItem(item);
    }

    void reset()
    {
        list->clear();
    }

    void elementWasRemoved(const HistoryElement *element)
    {
        for (int i=0; i<list->count(); )
        {
            if (list->item(i)->data(Qt::UserRole).value<HistoryElement*>() == element)
            {
                delete list->takeItem(i);
            }
            else
            {
                i++;
            }
        }
    }

protected:

    QListWidget* list;
};

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

class HistoryColorProvider : public RoleDataProvider
{
public:
    virtual QVariant data(const PatientModel*, const QModelIndex&, const Patient::Ptr& p)
    {
        QHash<int,QVariant>::const_iterator it = hash.find(p->id);
        if (it != hash.end())
        {
            return it.value();
        }
        const Disease& disease = p->firstDisease();
        CurrentStateIterator cs(disease.history());
        QColor c = VisualHistoryWidget::colorForState(cs.effectiveState());
        if (c.isValid())
        {
            if (c == QColor(Qt::white))
            {
                c = Qt::lightGray;
            }
            c.setAlpha(100);
            hash[p->id] = c;
            return c;
        }
        else
        {
            hash[p->id] = QVariant();
            return QVariant();
        }
    }
    QHash<int,QVariant> hash;
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
          lastDocumentation(0),
          lastDocumentationDate(0),
          lastValidation(0),
          lastValidationDate(0),
          proofOutput(0),
          colorProvider(0),
          proofReader(0),
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
    QCheckBox              *lastDocumentation;
    QDateEdit              *lastDocumentationDate;
    QCheckBox              *lastValidation;
    QDateEdit              *lastValidationDate;
    QListWidget            *proofOutput;
    HistoryColorProvider   *colorProvider;
    HistoryWindowProofReadOutput* proofReader;

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
    delete d->colorProvider;
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
        bool changed = false;

        Disease& disease = d->currentPatient->firstDisease();
        DiseaseHistory history = d->historyModel->history();
        DiseaseHistory previousHistory = disease.history();
        history.sort();

        // update history properties
        QDate lastDoc = history.lastDocumentation();
        if (d->lastDocumentation->isChecked())
        {
            if (lastDoc != d->lastDocumentationDate->date())
            {
                changed = true;
                history.setLastDocumentation(d->lastDocumentationDate->date());
            }
        }
        else
        {
            if (lastDoc.isValid())
            {
                changed = true;
                history.setLastDocumentation(QDate());
            }
        }

        QDate lastValid = history.lastValidation();
        if (d->lastValidation->isChecked())
        {
            if (lastValid != d->lastValidationDate->date())
            {
                changed = true;
                history.setLastValidation(d->lastValidationDate->date());
            }
        }
        else
        {
            if (lastValid.isValid())
            {
                changed = true;
                history.setLastValidation(QDate());
            }
        }

        if (changed || history != previousHistory)
        {
            /*qDebug() << "history changed from";
            qDebug() << previousHistory.toXml();
            qDebug() << "to new history";
            qDebug() << history.toXml();*/
            disease.setHistory(history);
            changed = true;
        }
        //qDebug() << "History of patient" << d->currentPatient->surname;
        //qDebug() << d->currentPatient->firstDisease().history().toXml();
        if (disease.initialDiagnosis != d->initialDiagnosisEdit->date())
        {
            disease.initialDiagnosis = d->initialDiagnosisEdit->date();
            changed = true;
        }
        if (disease.initialTNM.toText() != d->tnmEdit->text())
        {
            disease.initialTNM.setTNM(d->tnmEdit->text());
            changed = true;
        }

        if (changed)
        {
            PatientManager::instance()->updateData(d->currentPatient,
                                                   PatientManager::ChangedDiseaseHistory |
                                                   PatientManager::ChangedDiseaseMetadata);
            PatientManager::instance()->historySecurityCopy(d->currentPatient, "history", d->currentPatient->firstDisease().history().toXml());

            d->colorProvider->hash.remove(d->currentPatient->id);
        }
        else
        {
            //qDebug() << "Data unchanged, skipping DB write operation";
        }
    }
}

void HistoryWindow::setCurrentPatient(Patient::Ptr p)
{
    applyData();
    setCurrentElement(0);
    d->currentPatient = p;
    d->initialDiagnosisEdit->setDate(QDate::currentDate());
    d->proofReader->reset();

    if (d->currentPatient)
    {
        const Disease& disease = d->currentPatient->firstDisease();
        DiseaseHistory history = disease.history();
        d->historyModel->setHistory(history);
        d->initialDiagnosisEdit->setDate(disease.initialDiagnosis);
        d->tnmEdit->setText(disease.initialTNM.toText());
        d->visualHistoryWidget->setHistory(history);
        QDate lastDoc = history.lastDocumentation();
        d->lastDocumentation->setChecked(lastDoc.isValid());
        d->lastDocumentationDate->setDate(lastDoc.isValid() ? lastDoc : QDate::currentDate());
        QDate lastValid = history.lastValidation();
        d->lastValidation->setChecked(lastValid.isValid());
        d->lastValidationDate->setDate(lastValid.isValid() ? lastValid : (lastDoc.isValid() ? lastDoc : (history.end().isValid() ? history.end() : QDate::currentDate())));
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
    //qDebug() << "history element activated" << elem;
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
        QModelIndex index = d->sortModel->mapFromSource(d->historyModel->index(d->currentElement));
        //d->historyView->expand(index);
        //d->historyView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
        d->historyView->setCurrentIndex(index);
        d->visualHistoryWidget->setCursor(d->currentElement->date);
    }
    else
    {
        d->historyView->setCurrentIndex(QModelIndex());
        d->visualHistoryWidget->setCursor(QDate());
    }
}

void HistoryWindow::clearCurrentElement()
{
    if (d->currentElement)
    {
        d->historyModel->takeElement(d->currentElement);
        d->proofReader->elementWasRemoved(d->currentElement);
        delete d->currentElement;
        d->currentElement = 0;
        delete d->currentWidget;
        d->currentWidget = 0;
        setCurrentElement(DiseaseHistoryModel::retrieveElement(d->historyView->currentIndex()));
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
    d->colorProvider = new HistoryColorProvider;
    adapter()->model()->installRoleDataProvider(Qt::BackgroundRole, d->colorProvider);
    connect(d->patientView, SIGNAL(currentChanged(Patient::Ptr)), this, SLOT(setCurrentPatient(Patient::Ptr)));
    connect(d->patientView, SIGNAL(currentChanged(Patient::Ptr)), this, SIGNAL(activated(Patient::Ptr)));

    // History Tree View
    QWidget* secondWidget = new QWidget;
    QVBoxLayout* secondWidgetLayout = new QVBoxLayout;
    d->historyModel = new DiseaseHistoryModel(this);
    d->historyView  = new QTreeView;
    d->sortModel    = new HistoryElementSortModel(this);
    d->sortModel->setSourceModel(d->historyModel);
    d->historyView->setModel(d->sortModel);
    d->sortModel->sort(0);
    connect(d->historyView, SIGNAL(clicked(QModelIndex)), this, SLOT(historyElementActivated(QModelIndex)));
    secondWidgetLayout->addWidget(d->historyView, 1);
    secondWidget->setLayout(secondWidgetLayout);
    connect(d->historyModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(slotHistoryChanged()));
    connect(d->historyModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), this, SLOT(slotHistoryAboutToChange()));
    connect(d->historyModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(slotHistoryChanged()));
    connect(d->historyModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)), this, SLOT(slotHistoryAboutToChange()));
    connect(d->historyModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(slotHistoryChanged()));

    d->proofOutput = new QListWidget;
    d->proofReader = new HistoryWindowProofReadOutput(d->proofOutput);
    connect(d->proofOutput, SIGNAL(itemClicked(QListWidgetItem*)),
            this, SLOT(proofItemClicked(QListWidgetItem*)));

    // Main (editing) panel
    QWidget* mainPanel = new QWidget;
    d->mainPanelLayout = new QVBoxLayout;
    d->mainPanelLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    QScrollArea* visualHistoryWidgetScrollArea = new QScrollArea;
    d->visualHistoryWidget = new VisualHistoryWidget;
    d->visualHistoryWidget->setProofReader(d->proofReader);
    visualHistoryWidgetScrollArea->setWidget(d->visualHistoryWidget);
    visualHistoryWidgetScrollArea->setWidgetResizable(true);
    visualHistoryWidgetScrollArea->setFrameStyle(QFrame::NoFrame);
    d->mainPanelLayout->addWidget(visualHistoryWidgetScrollArea);
    connect(d->visualHistoryWidget, (void (VisualHistoryWidget::*)(HistoryElement*))(&VisualHistoryWidget::clicked), this, &HistoryWindow::visualHistoryClicked);

    d->patientDisplay = new PatientDisplay;
    d->patientDisplay->setShowGender(false);
    d->mainPanelLayout->addWidget(d->patientDisplay);

    QFormLayout* idAndTnmLayout = new QFormLayout;
    idAndTnmLayout->setLabelAlignment(Qt::AlignLeft);
    d->initialDiagnosisEdit = new QDateEdit;
    idAndTnmLayout->addRow(tr("Erstdiagnose:"), d->initialDiagnosisEdit);
    d->tnmEdit = new QLineEdit;
    idAndTnmLayout->addRow(tr("TNM (initial):"), d->tnmEdit);

    d->lastDocumentation = new QCheckBox(tr("Letzte Doku"));
    d->lastDocumentationDate = new QDateEdit;
    d->lastDocumentationDate->setEnabled(false);
    connect(d->lastDocumentation, SIGNAL(toggled(bool)), d->lastDocumentationDate, SLOT(setEnabled(bool)));
    connect(d->lastDocumentation, SIGNAL(toggled(bool)), this, SLOT(slotLastDocumentationDateChanged()));
    connect(d->lastDocumentationDate, SIGNAL(dateChanged(QDate)), this, SLOT(slotLastDocumentationDateChanged()));
    idAndTnmLayout->addRow(d->lastDocumentation, d->lastDocumentationDate);

    d->lastValidation = new QCheckBox(tr("Validiert"));
    d->lastValidationDate = new QDateEdit;
    d->lastValidationDate->setEnabled(false);
    connect(d->lastValidation, SIGNAL(toggled(bool)), d->lastValidationDate, SLOT(setEnabled(bool)));
    idAndTnmLayout->addRow(d->lastValidation, d->lastValidationDate);

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

    d->mainPanelLayout->addWidget(d->proofOutput);

    mainPanel->setLayout(d->mainPanelLayout);
    QScrollArea* scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
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
    //qDebug() << "Current element changed";
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
    //qDebug() << "Current therapy element changed";
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
    d->proofReader->elementWasRemoved(te);
    delete te;
}

void HistoryWindow::proofItemClicked(QListWidgetItem *item)
{
    HistoryElement* e = item->data(Qt::UserRole).value<HistoryElement*>();
    if (!e)
    {
        return;
    }
    QModelIndex index = d->sortModel->mapFromSource(d->historyModel->index(e));
    d->patientView->setCurrentIndex(index);
    historyElementActivated(index);
}

void HistoryWindow::slotHistoryAboutToChange()
{
    d->visualHistoryWidget->setHistory(DiseaseHistory());
}

void HistoryWindow::slotHistoryChanged()
{
    d->visualHistoryWidget->setHistory(d->historyModel->history());
    if (d->currentElement)
    {
        d->visualHistoryWidget->setCursor(d->currentElement->date);
    }
}

void HistoryWindow::slotLastDocumentationDateChanged()
{
    d->visualHistoryWidget->updateLastDocumentation(
                d->lastDocumentation->isChecked() ? d->lastDocumentationDate->date() : QDate());
    if (d->lastDocumentation->isChecked() && !d->lastValidation->isChecked())
    {
        d->lastValidationDate->setDate(d->lastDocumentationDate->date());
    }
}

void HistoryWindow::visualHistoryClicked(HistoryElement* e)
{
    if (e)
    {
        setCurrentElement(e);
    }
}

void HistoryWindow::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_F10)
    {
        QModelIndex next = d->patientView->indexBelow(d->patientView->currentIndex());
        if (next.isValid())
        {
            d->patientView->setCurrentIndex(next);
        }
    }
    if (e->key() == Qt::Key_F9)
    {
        QModelIndex prev = d->patientView->indexAbove(d->patientView->currentIndex());
        if (prev.isValid())
        {
            d->patientView->setCurrentIndex(prev);
        }
    }
}
