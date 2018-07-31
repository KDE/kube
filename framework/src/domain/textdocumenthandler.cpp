/*
    Copyright (c) 2017 Christian Mollekopf <mollekopf@kolabsystems.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/
#include "textdocumenthandler.h"

#include <QQuickTextDocument>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QDebug>

TextDocumentHandler::TextDocumentHandler(QObject *parent)
    : QObject(parent),
    mDocument(nullptr),
    mCursorPosition(-1),
    mSelectionStart(0),
    mSelectionEnd(0)
{
}

bool TextDocumentHandler::containsFormatting() const
{
    if (mDocument) {
        for (const auto &format : mDocument->textDocument()->allFormats()) {
            switch(format.type()) {
                case QTextFormat::CharFormat: {
                    const auto charFormat = format.toCharFormat();
                    if (charFormat.fontWeight() != QFont::Normal) {
                        return true;
                    }
                    if (charFormat.fontItalic()) {
                        return true;
                    }
                    if (charFormat.fontUnderline()) {
                        return true;
                    }
                    break;
                }
                case QTextFormat::BlockFormat:
                case QTextFormat::FrameFormat:
                default:
                    break;
            }
        }
    }
    return false;
}

void TextDocumentHandler::resetFormat()
{
    if (mDocument) {
        mDocument->textDocument()->setPlainText(mDocument->textDocument()->toPlainText());
    }
    mCachedTextFormat = {};
    reset();
}

QQuickTextDocument *TextDocumentHandler::document() const
{
    return mDocument;
}

void TextDocumentHandler::setDocument(QQuickTextDocument *document)
{
    if (document != mDocument) {
        mDocument = document;
        connect(mDocument->textDocument(), &QTextDocument::contentsChange, this, &TextDocumentHandler::contentsChange);
        emit documentChanged();
        emit textChanged();
    }
}

QString TextDocumentHandler::text() const
{
    if (containsFormatting()) {
        return htmlText();
    }
    return plainText();
}

QString TextDocumentHandler::plainText() const
{
    if (mDocument) {
        return mDocument->textDocument()->toPlainText();
    }
    return {};
}

QString TextDocumentHandler::htmlText() const
{
    if (mDocument) {
        return mDocument->textDocument()->toHtml();
    }
    return {};
}

int TextDocumentHandler::cursorPosition() const
{
    return mCursorPosition;
}

void TextDocumentHandler::setCursorPosition(int position)
{
    if (position != mCursorPosition) {
        mCursorPosition = position;
        reset();
        emit cursorPositionChanged();
    }
}

int TextDocumentHandler::selectionStart() const
{
    return mSelectionStart;
}

void TextDocumentHandler::setSelectionStart(int position)
{
    if (position != mSelectionStart) {
        mSelectionStart = position;
        emit selectionStartChanged();
    }
}

int TextDocumentHandler::selectionEnd() const
{
    return mSelectionEnd;
}

void TextDocumentHandler::setSelectionEnd(int position)
{
    if (position != mSelectionEnd) {
        mSelectionEnd = position;
        emit selectionEndChanged();
    }
}

QTextCharFormat TextDocumentHandler::charFormat() const
{
    if (mCachedTextFormat) {
        return *mCachedTextFormat;
    }
    auto cursor = textCursor();
    if (cursor.isNull()) {
        return {};
    }
    return cursor.charFormat();
}

QString TextDocumentHandler::fontFamily() const
{
    return charFormat().font().family();
}

void TextDocumentHandler::setFontFamily(const QString &family)
{
    QTextCharFormat format;
    format.setFontFamily(family);
    mergeFormatOnWordOrSelection(format);
    emit fontFamilyChanged();
}

QColor TextDocumentHandler::textColor() const
{
    return charFormat().foreground().color();
}

void TextDocumentHandler::setTextColor(const QColor &color)
{
    QTextCharFormat format;
    format.setForeground(QBrush(color));
    mergeFormatOnWordOrSelection(format);
    emit textColorChanged();
}

Qt::Alignment TextDocumentHandler::alignment() const
{
    auto cursor = textCursor();
    if (cursor.isNull()) {
        return Qt::AlignLeft;
    }
    return cursor.blockFormat().alignment();
}

void TextDocumentHandler::setAlignment(Qt::Alignment alignment)
{
    QTextBlockFormat format;
    format.setAlignment(alignment);
    QTextCursor cursor = textCursor();
    cursor.mergeBlockFormat(format);
    emit alignmentChanged();
}

bool TextDocumentHandler::bold() const
{
    return charFormat().fontWeight() == QFont::Bold;
}

void TextDocumentHandler::setBold(bool bold)
{
    QTextCharFormat format;
    format.setFontWeight(bold ? QFont::Bold : QFont::Normal);
    mergeFormatOnWordOrSelection(format);
    emit boldChanged();
}

bool TextDocumentHandler::italic() const
{
    return charFormat().fontItalic();
}

void TextDocumentHandler::setItalic(bool italic)
{
    QTextCharFormat format;
    format.setFontItalic(italic);
    mergeFormatOnWordOrSelection(format);
    emit italicChanged();
}

bool TextDocumentHandler::underline() const
{
    return charFormat().fontUnderline();
}

void TextDocumentHandler::setUnderline(bool underline)
{
    QTextCharFormat format;
    format.setFontUnderline(underline);
    mergeFormatOnWordOrSelection(format);
    emit underlineChanged();
}

int TextDocumentHandler::fontSize() const
{
    return charFormat().font().pointSize();
}

void TextDocumentHandler::setFontSize(int size)
{
    if (size <= 0)
        return;

    if (charFormat().property(QTextFormat::FontPointSize).toInt() == size)
        return;

    QTextCharFormat format;
    format.setFontPointSize(size);
    mergeFormatOnWordOrSelection(format);
    emit fontSizeChanged();
}

void TextDocumentHandler::reset()
{
    emit fontFamilyChanged();
    emit alignmentChanged();
    emit boldChanged();
    emit italicChanged();
    emit underlineChanged();
    emit fontSizeChanged();
    emit textColorChanged();
}

QTextCursor TextDocumentHandler::textCursor() const
{
    if (mDocument) {
        if (QTextDocument *doc = mDocument->textDocument()) {
            QTextCursor cursor = QTextCursor(doc);
            if (mSelectionStart != mSelectionEnd) {
                cursor.setPosition(mSelectionStart);
                cursor.setPosition(mSelectionEnd, QTextCursor::KeepAnchor);
            } else {
                cursor.setPosition(mCursorPosition);
            }
            return cursor;
        }
    }
    return QTextCursor();
}

void TextDocumentHandler::contentsChange(int position, int charsRemoved, int charsAdded)
{
    Q_UNUSED(charsRemoved)
    if (mCachedTextFormat) {
        if (charsAdded) {
            //Apply cached formatting
            QTextCursor cursor = textCursor();
            cursor.setPosition(position + charsAdded, QTextCursor::KeepAnchor);
            cursor.mergeCharFormat(*mCachedTextFormat);
            //This is somehow necessary, otherwise space can break in the editor.
            cursor.setPosition(position + charsAdded, QTextCursor::MoveAnchor);
        }
        mCachedTextFormat = {};
    }
    emit textChanged();
}

void TextDocumentHandler::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = textCursor();

    if (cursor.hasSelection()) {
        cursor.mergeCharFormat(format);
    } else {
        if (mCachedTextFormat) {
            mCachedTextFormat->merge(format);
        } else {
            //If we have nothing to format right now we cache the result until the next char is inserted.
            mCachedTextFormat = QSharedPointer<QTextCharFormat>::create(format);
        }
    }
}


bool TextDocumentHandler::isHtml(const QString &text)
{
    return Qt::mightBeRichText(text);
}
