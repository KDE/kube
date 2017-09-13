/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "documenthandler.h"

#include <QFile>
#include <QFileInfo>
#include <QFileSelector>
#include <QQmlFile>
#include <QQmlFileSelector>
#include <QQuickTextDocument>
#include <QTextCharFormat>
#include <QTextCodec>
#include <QTextDocument>
#include <QDebug>

DocumentHandler::DocumentHandler(QObject *parent)
    : QObject(parent)
    , m_document(nullptr)
    , m_cursorPosition(-1)
    , m_selectionStart(0)
    , m_selectionEnd(0)
{
}

QQuickTextDocument *DocumentHandler::document() const
{
    return m_document;
}

void DocumentHandler::setDocument(QQuickTextDocument *document)
{
    if (document != m_document) {
        m_document = document;
        emit documentChanged();
    }
}

int DocumentHandler::cursorPosition() const
{
    return m_cursorPosition;
}

void DocumentHandler::setCursorPosition(int position)
{
    if (position != m_cursorPosition) {
        //Skip over invisible char
        if (m_cursorPosition < position) {
            auto cursor = textCursor();
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
            if (cursor.selectedText() == "\u2063") {
                position++;
            }
        }
        if (m_cursorPosition > position) {
            auto cursor = textCursor();
            cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
            if (cursor.selectedText() == "\u2063") {
                position--;
            }
        }
        m_cursorPosition = position;
        reset();
        emit cursorPositionChanged();
    }
}

int DocumentHandler::selectionStart() const
{
    return m_selectionStart;
}

void DocumentHandler::setSelectionStart(int position)
{
    if (position != m_selectionStart) {
        m_selectionStart = position;
        emit selectionStartChanged();
    }
}

int DocumentHandler::selectionEnd() const
{
    return m_selectionEnd;
}

void DocumentHandler::setSelectionEnd(int position)
{
    if (position != m_selectionEnd) {
        m_selectionEnd = position;
        emit selectionEndChanged();
    }
}

QString DocumentHandler::fontFamily() const
{
    auto cursor = textCursor();
    if (cursor.isNull()) {
        return QString();
    }
    return cursor.charFormat().font().family();
}

void DocumentHandler::setFontFamily(const QString &family)
{
    QTextCharFormat format;
    format.setFontFamily(family);
    mergeFormatOnWordOrSelection(format);
    emit fontFamilyChanged();
}

QColor DocumentHandler::textColor() const
{
    auto cursor = textCursor();
    if (cursor.isNull()) {
        return QColor(Qt::black);
    }
    return cursor.charFormat().foreground().color();
}

void DocumentHandler::setTextColor(const QColor &color)
{
    QTextCharFormat format;
    format.setForeground(QBrush(color));
    mergeFormatOnWordOrSelection(format);
    emit textColorChanged();
}

Qt::Alignment DocumentHandler::alignment() const
{
    auto cursor = textCursor();
    if (cursor.isNull()) {
        return Qt::AlignLeft;
    }
    return cursor.blockFormat().alignment();
}

void DocumentHandler::setAlignment(Qt::Alignment alignment)
{
    QTextBlockFormat format;
    format.setAlignment(alignment);
    QTextCursor cursor = textCursor();
    cursor.mergeBlockFormat(format);
    emit alignmentChanged();
}

bool DocumentHandler::bold() const
{
    auto cursor = textCursor();
    if (cursor.isNull()) {
        return false;
    }
    return cursor.charFormat().fontWeight() == QFont::Bold;
}

void DocumentHandler::setBold(bool bold)
{
    QTextCharFormat format;
    format.setFontWeight(bold ? QFont::Bold : QFont::Normal);
    mergeFormatOnWordOrSelection(format);
    emit boldChanged();
}

bool DocumentHandler::italic() const
{
    auto cursor = textCursor();
    if (cursor.isNull()) {
        return false;
    }
    return cursor.charFormat().fontItalic();
}

void DocumentHandler::setItalic(bool italic)
{
    QTextCharFormat format;
    format.setFontItalic(italic);
    mergeFormatOnWordOrSelection(format);
    emit italicChanged();
}

bool DocumentHandler::underline() const
{
    auto cursor = textCursor();
    if (cursor.isNull()) {
        return false;
    }
    return cursor.charFormat().fontUnderline();
}

void DocumentHandler::setUnderline(bool underline)
{
    QTextCharFormat format;
    format.setFontUnderline(underline);
    mergeFormatOnWordOrSelection(format);
    emit underlineChanged();
}

int DocumentHandler::fontSize() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return 0;
    QTextCharFormat format = cursor.charFormat();
    return format.font().pointSize();
}

void DocumentHandler::setFontSize(int size)
{
    if (size <= 0)
        return;

    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return;

    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);

    if (cursor.charFormat().property(QTextFormat::FontPointSize).toInt() == size)
        return;

    QTextCharFormat format;
    format.setFontPointSize(size);
    mergeFormatOnWordOrSelection(format);
    emit fontSizeChanged();
}

void DocumentHandler::reset()
{
    emit fontFamilyChanged();
    emit alignmentChanged();
    emit boldChanged();
    emit italicChanged();
    emit underlineChanged();
    emit fontSizeChanged();
    emit textColorChanged();
}

QTextCursor DocumentHandler::textCursor() const
{
    if (QTextDocument *doc = textDocument()) {
        QTextCursor cursor = QTextCursor(doc);
        if (m_selectionStart != m_selectionEnd) {
            cursor.setPosition(m_selectionStart);
            cursor.setPosition(m_selectionEnd, QTextCursor::KeepAnchor);
        } else {
            cursor.setPosition(m_cursorPosition);
        }
        return cursor;
    }
    return QTextCursor();
}

QTextDocument *DocumentHandler::textDocument() const
{
    if (!m_document) {
        return nullptr;
    }
    return m_document->textDocument();
}

void DocumentHandler::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = textCursor();

    cursor.mergeCharFormat(format);

    /*
     * FIXME: This is a fantastic hack to change the text format on the TextArea's internal cursor.
     * The TextArea internally listens for the contentChanged signal of the document and then simply
     * copies the format with QTextCursor::charFormat. This does of course not work without any text in
     * said format. The Qt Example solution to the problem was to select the current word, which is both not
     * the behaviour that we want, nor does it work if you don't currently have a word under the cursor.
     * We therefore have to insert an invisible character to transport the format, which we have to clear out in the end again.
     */
    if (!cursor.hasSelection()) {
        cursor.insertText("\u2063");
    }
}
