/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 03.08.2015
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

#include "rawtextenterpage.h"

#include <QApplication>
#include <QClipboard>
#include <QPushButton>
#include <QTextCursor>
#include <QVBoxLayout>

RawTextEnterPage::RawTextEnterPage()
{
    setTitle(tr("Text eingaben"));
    setSubTitle(tr("Es können Pathologiebefunde (Textversion, nicht PDF) eines oder mehrerer Patienten eingegeben werden"));

    QVBoxLayout* layout = new QVBoxLayout;

    textEdit = new QTextEdit;
    layout->addWidget(textEdit, 1);
    registerField("rawText*", textEdit, "plainText", SIGNAL(textChanged()));

    QPushButton* clipboardButton = new QPushButton(tr("Aus Zwischenablage einfügen"));
    layout->addWidget(clipboardButton);

    setLayout(layout);

    connect(clipboardButton, &QPushButton::clicked, this, &RawTextEnterPage::copyClipboard);
}

void RawTextEnterPage::copyClipboard()
{
    QClipboard* cb = QApplication::clipboard();
    QTextCursor cursor(textEdit->textCursor());
    QString text = cb->text();
    if (!text.endsWith("\n\n"))
    {
        if (text.endsWith('\n'))
        {
            text += '\n';
        }
        else
        {
            text += "\n\n";
        }
    }
    textEdit->insertPlainText(text);
    cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    textEdit->setTextCursor(cursor);
}
