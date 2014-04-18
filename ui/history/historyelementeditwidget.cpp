/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 23.05.2013
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

#include "historyelementeditwidget.h"

// Qt includes

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

// Local includes

#include "datevalidator.h"
#include "therapyelementeditwidget.h"


static void addButton(int type, const QString& text, QButtonGroup* group,
                      QGridLayout* layout, int r, int c)
{
    QRadioButton* b = new QRadioButton(text);
    group->addButton(b, type);
    layout->addWidget(b, r, c);
}

static void addFlagButton(int type, const QString& text, QButtonGroup* group,
                          QGridLayout* layout, int r, int c)
{
    QCheckBox* b = new QCheckBox(text);
    group->addButton(b, type);
    layout->addWidget(b, r, c);
}

static void clearButtonGroup(QButtonGroup* group)
{
    if (group->checkedButton())
    {
        bool exclusive = group->exclusive();
        group->setExclusive(false);
        group->checkedButton()->setChecked(false);
        group->setExclusive(exclusive);
    }
}

static void setCheckedSafe(QButtonGroup* group, int id)
{
    QAbstractButton* button = group->button(id);
    if (button)
    {
        button->setChecked(true);
    }
    else
    {
        clearButtonGroup(group);
    }
}

template <typename Flags>
static void setCheckedFlags(QButtonGroup* group, Flags flags)
{
    foreach (QAbstractButton* b, group->buttons())
    {
        b->setChecked(flags & group->id(b));
    }
}



/*static void setCheckedSafe(QComboBox* box, int id)
{
    int index = box->findData(id);
    box->setCurrentIndex(index);
}*/

template <typename E>
static E checkedValue(QButtonGroup* group, E defaultId)
{
    int id = group->checkedId();
    if (id == -1)
    {
        return defaultId;
    }
    return (E)id;
}

template <typename E>
static E checkedValue(QComboBox* box, E defaultId)
{
    if (box->currentIndex() == -1)
    {
        return defaultId;
    }
    QVariant data = box->itemData(box->currentIndex());
    if (data.isValid())
    {
        return (E)data.toInt();
    }
    return defaultId;
}

template <typename E>
static QFlags<E> checkedFlags(QButtonGroup* group)
{
    QFlags<E> flags;
    foreach (QAbstractButton* b, group->buttons())
    {
        if (b->isChecked())
        {
            flags |= (E)group->id(b);
        }
    }
    return flags;
}

DateCommentWidget::DateCommentWidget()
{
    formLayout = new QFormLayout;
    firstDateEdit   = new QLineEdit;
    endDateEdit     = new QLineEdit;
    commentLineEdit = new QLineEdit;
    DateValidator* firstDateValidator = new DateValidator(this);
    firstDateValidator->setKeinJahrEnabled(true);
    firstDateValidator->setLineEdit(firstDateEdit);
    DateValidator* secondDateValidator = new DateValidator(this);
    secondDateValidator->setKeinJahrEnabled(true);
    secondDateValidator->setLineEdit(endDateEdit);
    formLayout->addRow(tr("Beginn"), firstDateEdit);
    formLayout->addRow(tr("Ende"), endDateEdit);
    formLayout->addRow(tr("Kommentar"), commentLineEdit);
    setLayout(formLayout);

    setDate(QDate());
    setEndDate(QDate());

    connect(firstDateEdit, SIGNAL(textChanged(QString)), this, SIGNAL(changed()));
    connect(endDateEdit, SIGNAL(textChanged(QString)), this, SIGNAL(changed()));
    connect(commentLineEdit, SIGNAL(textEdited(QString)), this, SIGNAL(changed()));
}

void DateCommentWidget::setMode(Mode mode)
{
    switch (mode)
    {
    case SingleDateMode:
        endDateEdit->setVisible(false);
        formLayout->labelForField(endDateEdit)->setVisible(false);
        static_cast<QLabel*>(formLayout->labelForField(firstDateEdit))->setText(tr("Datum"));
        break;
    case DateRangeMode:
        endDateEdit->setVisible(true);
        formLayout->labelForField(endDateEdit)->setVisible(true);
        static_cast<QLabel*>(formLayout->labelForField(firstDateEdit))->setText(tr("Beginn"));
        break;
    }
}

void DateCommentWidget::setCommentEditVisible(bool visible)
{
    commentLineEdit->setVisible(visible);
    formLayout->labelForField(commentLineEdit)->setVisible(visible);
}

QDate DateCommentWidget::date() const
{
    return DateValidator::TextToDate(firstDateEdit->text());
}

QDate DateCommentWidget::endDate() const
{
    return DateValidator::TextToDate(endDateEdit->text());
}

QString DateCommentWidget::comment() const
{
    return commentLineEdit->text();
}

void DateCommentWidget::setDate(const QDate& date)
{
    DateValidator::setDate(firstDateEdit, date);
}

void DateCommentWidget::setEndDate(const QDate& date)
{
    DateValidator::setDate(endDateEdit, date);
}

void DateCommentWidget::setComment(const QString &comment)
{
    commentLineEdit->setText(comment);
}

// ----

HistoryElementEditWidget::HistoryElementEditWidget(QWidget *parent)
    : QWidget(parent),
      m_element(0)
{
    mainLayout = new QVBoxLayout;
    dateCommentWidget = new DateCommentWidget;
    mainLayout->addWidget(dateCommentWidget);
    setLayout(mainLayout);

    connect(dateCommentWidget, SIGNAL(changed()), this, SIGNAL(changed()));
}

HistoryElementEditWidget::~HistoryElementEditWidget()
{
}

HistoryElement* HistoryElementEditWidget::applyToElement() const
{
    return m_element;
}

HistoryElement* HistoryElementEditWidget::element() const
{
    return m_element;
}

void HistoryElementEditWidget::setElement(HistoryElement* e)
{
    m_element = e;
}

HistoryElementEditWidget* HistoryElementEditWidget::create(HistoryElement* e)
{
    if (e->is<Therapy>())
    {
        TherapyEditWidget* widget = new TherapyEditWidget;
        widget->setElement(e);
        return widget;
    }
    else if (e->is<Finding>())
    {
        FindingEditWidget* widget = new FindingEditWidget;
        widget->setElement(e);
        return widget;
    }
    else if (e->is<DiseaseState>())
    {
        DiseaseStateEditWidget* widget = new DiseaseStateEditWidget;
        widget->setElement(e);
        return widget;
    }
    return 0;
}

// ----

TherapyEditWidget::TherapyEditWidget()
{
    dateCommentWidget->setMode(DateCommentWidget::DateRangeMode);
    dateCommentWidget->setCommentEditVisible(true);
    elementWidgetLayout = new QVBoxLayout;

    mainLayout->addLayout(elementWidgetLayout);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    moreCTxButton = new QPushButton(QIcon::fromTheme("add"), tr("Weitere Substanz"));
    connect(moreCTxButton, SIGNAL(clicked()), this, SLOT(addSubstance()));
    buttonLayout->addWidget(moreCTxButton);
    moreRTxButton = new QPushButton(QIcon::fromTheme("add"), tr("Bestrahlung"));
    connect(moreRTxButton, SIGNAL(clicked()), this, SLOT(addRadiation()));
    buttonLayout->addWidget(moreRTxButton);
    moreToxButton = new QPushButton(QIcon::fromTheme("add"), tr("Toxizität"));
    connect(moreToxButton, SIGNAL(clicked()), this, SLOT(addToxicity()));
    buttonLayout->addWidget(moreToxButton);
    mainLayout->addLayout(buttonLayout);

    QGroupBox* infoBox = new QGroupBox(tr("Zusätzliche Aspekte"));
    infoGroup = new QButtonGroup;
    infoGroup->setExclusive(false);
    QGridLayout* infoLayout = new QGridLayout;
    addFlagButton(Therapy::BeginsTherapyBlock, tr("Anfang Therapieblock"), infoGroup, infoLayout, 0, 0);
    addFlagButton(Therapy::EndsTherapyBlock, tr("Ende Therapieblock"), infoGroup, infoLayout, 1, 0);
    infoBox->setLayout(infoLayout);
    mainLayout->addWidget(infoBox);
}

HistoryElement* TherapyEditWidget::applyToElement() const
{
    Therapy* t     = m_element->as<Therapy>();
    t->date        = dateCommentWidget->beginDate();
    t->description = dateCommentWidget->comment();
    t->end         = dateCommentWidget->endDate();
    t->additionalInfos = checkedFlags<Therapy::AdditionalInfo>(infoGroup);

    t->elements.clear();
    foreach (TherapyElementEditWidget* editWidget, elementWidgets)
    {
        t->elements << editWidget->applyToElement();
    }
    return t;
}

void TherapyEditWidget::setElement(HistoryElement* element)
{
    HistoryElementEditWidget::setElement(element);
    Therapy* t = m_element->as<Therapy>();
    dateCommentWidget->setBeginDate(t->date);
    dateCommentWidget->setEndDate(t->end);
    dateCommentWidget->setComment(t->description);
    setCheckedFlags(infoGroup, t->additionalInfos);

    switch (t->type)
    {
    case Therapy::CTx:
        moreRTxButton->setVisible(false);
        moreCTxButton->setVisible(true);
        break;
    case Therapy::RTx:
        moreCTxButton->setVisible(false);
        moreRTxButton->setVisible(true);
        break;
    case Therapy::RCTx:
        moreCTxButton->setVisible(true);
        moreRTxButton->setVisible(true);
        break;
    default:
        moreCTxButton->setVisible(false);
        moreRTxButton->setVisible(false);
        break;
    }

    foreach (TherapyElement* elem, t->elements)
    {
        addElementUI(elem);
    }
}

void TherapyEditWidget::addElement(TherapyElement* te)
{
    emit addTherapyElement(te);
    addElementUI(te);
}

void TherapyEditWidget::addElementUI(TherapyElement* te)
{
    TherapyElementEditWidget* widget = TherapyElementEditWidget::create(te);
    if (!widget)
    {
        qDebug() << "No TherapyElementEditWidget implemented for TherapyElement" << te;
        return;
    }
    elementWidgets << widget;
    elementWidgetLayout->addWidget(widget);
    connect(widget, SIGNAL(changed()), this, SLOT(slotTherapyElementChanged()));
    connect(widget, SIGNAL(remove()), this, SLOT(slotTherapyElementRemove()));
}

void TherapyEditWidget::removeElementUI(TherapyElement* te)
{
    foreach (TherapyElementEditWidget* editWidget, elementWidgets)
    {
        if (editWidget->element() == te)
        {
            elementWidgets.removeAll(editWidget);
            delete editWidget;
        }
    }
}

QString TherapyEditWidget::heading() const
{
    return Therapy::uiLabel(m_element->as<Therapy>()->type);
}

void TherapyEditWidget::addSubstance()
{
    addElement(new Chemotherapy);
}

void TherapyEditWidget::addRadiation()
{
    addElement(new Radiotherapy);
}

void TherapyEditWidget::addToxicity()
{
    addElement(new Toxicity);
}

void TherapyEditWidget::slotTherapyElementChanged()
{
    TherapyElementEditWidget* w = qobject_cast<TherapyElementEditWidget*>(sender());
    if (!w)
    {
        return;
    }
    emit therapyElementChanged(w->applyToElement());
}

void TherapyEditWidget::slotTherapyElementRemove()
{
    TherapyElementEditWidget* w = qobject_cast<TherapyElementEditWidget*>(sender());
    qDebug() << "slotTherapyElementRemove" << w;
    if (!w)
    {
        return;
    }
    emit therapyElementRemove(w->element());
}

// ----


FindingEditWidget::FindingEditWidget()
{
    dateCommentWidget->setMode(DateCommentWidget::SingleDateMode);
    dateCommentWidget->setCommentEditVisible(true);

    QGroupBox* modalityBox = new QGroupBox;
    modalityGroup   = new QButtonGroup;
    QGridLayout* modalityLayout = new QGridLayout;

    addButton(Finding::CT, tr("CT"), modalityGroup, modalityLayout, 0, 0);
    addButton(Finding::MRI, tr("MRT"), modalityGroup, modalityLayout, 0, 1);
    addButton(Finding::Sono, tr("Sono"), modalityGroup, modalityLayout, 0, 2);
    addButton(Finding::Clinical, tr("Klinisch"), modalityGroup, modalityLayout, 1, 0);
    addButton(Finding::XRay, tr("Röntgen"), modalityGroup, modalityLayout, 1, 1);
    addButton(Finding::PETCT, tr("PET-CT"), modalityGroup, modalityLayout, 1, 2);
    addButton(Finding::Scintigraphy, tr("Szinti"), modalityGroup, modalityLayout, 2, 0);
    addButton(Finding::Histopathological, tr("Histo"), modalityGroup, modalityLayout, 2, 1);
    addButton(Finding::Death, tr("Tod"), modalityGroup, modalityLayout, 2, 2);
    modalityBox->setLayout(modalityLayout);

    QGroupBox* contextBox = new QGroupBox(tr("Im Rahmen von"));
    contextGroup = new QButtonGroup;
    QGridLayout* contextLayout = new QGridLayout;
    addButton(Finding::Antecedent, tr("Vor Erstdiagnose"), contextGroup, contextLayout, 0, 1);
    addButton(Finding::InitialDiagnosis, tr("Erstdiagnose / Staging"), contextGroup, contextLayout, 0, 0);
    addButton(Finding::ResponseEvaluation, tr("Therapie / Restaging"),
              contextGroup, contextLayout, 1, 0);
    addButton(Finding::FollowUp, tr("Nachsorge"), contextGroup, contextLayout, 1, 1);
    contextBox->setLayout(contextLayout);

    QGroupBox* resultBox = new QGroupBox(tr("Ergebnis"));
    resultGroup          = new QButtonGroup;
    QGridLayout* resultGroupLayout = new QGridLayout;

    addButton(Finding::ProgressiveDisease, tr("Progress"), resultGroup, resultGroupLayout, 0, 0);
    addButton(Finding::StableDisease, tr("Stabile Erkrankung"), resultGroup, resultGroupLayout, 1, 0);
    addButton(Finding::MinorResponse, tr("Geringes Ansprechen"), resultGroup, resultGroupLayout, 2, 0);
    addButton(Finding::PartialResponse, tr("Partielle Remission"), resultGroup, resultGroupLayout, 3, 0);
    addButton(Finding::CompleteResponse, tr("Komplettremission"), resultGroup, resultGroupLayout, 4, 0);
    addButton(Finding::NoEvidenceOfDisease, tr("Kein Erkrankungsnachweis"), resultGroup, resultGroupLayout, 5, 0);
    addButton(Finding::InitialFindingResult, tr("Erstbefund"), resultGroup, resultGroupLayout, 6, 0);
    addButton(Finding::Recurrence, tr("Rezidiv"), resultGroup, resultGroupLayout, 7, 0);
    addButton(Finding::ResultNotApplicable, tr("(kein Ergebnis)"), resultGroup, resultGroupLayout, 8, 0);
    resultBox->setLayout(resultGroupLayout);

    QGroupBox* infoBox = new QGroupBox(tr("Zusätzliche Aspekte"));
    infoGroup = new QButtonGroup;
    infoGroup->setExclusive(false);
    QGridLayout* infoLayout = new QGridLayout;
    addFlagButton(Finding::LocalRecurrence, tr("Lokalrezidiv"), infoGroup, infoLayout, 0, 0);
    addFlagButton(Finding::Metastasis, tr("Metastasierung"), infoGroup, infoLayout, 0, 1);
    addFlagButton(Finding::CentralNervous, tr("ZNS"), infoGroup, infoLayout, 0, 2);
    infoBox->setLayout(infoLayout);

    mainLayout->addWidget(modalityBox);
    mainLayout->addWidget(contextBox);
    mainLayout->addWidget(resultBox);
    mainLayout->addWidget(infoBox);

    connect(modalityGroup, SIGNAL(buttonClicked(int)), this, SIGNAL(changed()));
    connect(contextGroup, SIGNAL(buttonClicked(int)), this, SIGNAL(changed()));
    connect(resultGroup, SIGNAL(buttonClicked(int)), this, SIGNAL(changed()));
}

HistoryElement* FindingEditWidget::applyToElement() const
{
    Finding* f = m_element->as<Finding>();
    f->date        = dateCommentWidget->beginDate();
    f->description = dateCommentWidget->comment();
    f->type = checkedValue<Finding::Type>(modalityGroup, Finding::UndefinedType);
    f->context = checkedValue<Finding::Context>(contextGroup, Finding::UndefinedContext);
    f->result = checkedValue<Finding::Result>(resultGroup, Finding::UndefinedResult);
    f->additionalInfos = checkedFlags<Finding::AdditionalInfo>(infoGroup);
    return HistoryElementEditWidget::applyToElement();
}

void FindingEditWidget::setElement(HistoryElement* element)
{
    HistoryElementEditWidget::setElement(element);

    Finding* f = m_element->as<Finding>();

    dateCommentWidget->setDate(f->date);
    dateCommentWidget->setComment(f->description);

    setCheckedSafe(modalityGroup, f->type);
    setCheckedSafe(contextGroup, f->context);
    setCheckedSafe(resultGroup, f->result);
    setCheckedFlags(infoGroup, f->additionalInfos);
}

// ----

DiseaseStateEditWidget::DiseaseStateEditWidget()
{
    dateCommentWidget->setMode(DateCommentWidget::SingleDateMode);
    dateCommentWidget->setCommentEditVisible(false);

    QGroupBox* statusBox = new QGroupBox(tr("Status"));
    statusGroup   = new QButtonGroup;
    QGridLayout* statusLayout = new QGridLayout;

    addButton(DiseaseState::InitialDiagnosis, tr("Erstdiagnose"), statusGroup, statusLayout, 0, 0);
    addButton(DiseaseState::Therapy, tr("Therapie"), statusGroup, statusLayout, 1, 0);
    addButton(DiseaseState::BestSupportiveCare, tr("Best supportive care"), statusGroup, statusLayout, 2, 0);
    addButton(DiseaseState::WatchAndWait, tr("Verlaufskontrolle"), statusGroup, statusLayout, 3, 0);
    addButton(DiseaseState::FollowUp, tr("Nachsorge"), statusGroup, statusLayout, 4, 0);
    addButton(DiseaseState::Deceased, tr("Verstorben"), statusGroup, statusLayout, 5, 0);
    addButton(DiseaseState::LossOfContact, tr("Kontakt abgebrochen"), statusGroup, statusLayout, 6, 0);
    addButton(DiseaseState::UnknownState, tr("Status unbekannt"), statusGroup, statusLayout, 7, 0);
    statusBox->setLayout(statusLayout);

    mainLayout->addWidget(statusBox);

    connect(statusGroup, SIGNAL(buttonClicked(int)), this, SIGNAL(changed()));
}

HistoryElement* DiseaseStateEditWidget::applyToElement() const
{
    DiseaseState* s = m_element->as<DiseaseState>();
    s->date         = dateCommentWidget->date();
    s->state        = checkedValue<DiseaseState::State>(statusGroup, DiseaseState::UnknownState);
    return HistoryElementEditWidget::applyToElement();
}

void DiseaseStateEditWidget::setElement(HistoryElement* element)
{
    HistoryElementEditWidget::setElement(element);

    DiseaseState* s = m_element->as<DiseaseState>();
    dateCommentWidget->setDate(s->date);
    setCheckedSafe(statusGroup, s->state);
}

