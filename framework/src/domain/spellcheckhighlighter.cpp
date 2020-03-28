/*
    Copyright (c) 2020 Christian Mollekopf <mollekopf@kolabsystems.com>

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
#include "spellcheckhighlighter.h"

#include <QDebug>
#include <QTextBoundaryFinder>

SpellcheckHighlighter::SpellcheckHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent),
    mSpellchecker{new Sonnet::Speller()},
    mLanguageGuesser{new Sonnet::GuessLanguage()}
{
    //Danger red from our color scheme
    mErrorFormat.setForeground(QColor{"#ed1515"});
    mQuoteFormat.setForeground(QColor{"#7f8c8d"});

    if (!mSpellchecker->isValid()) {
        qWarning() << "Spellchecker is invalid";
    }
    qDebug() << "Available dictionaries: " << mSpellchecker->availableDictionaries();
}

static QVector<QStringRef> split(QTextBoundaryFinder::BoundaryType boundary, const QString &text)
{
    QVector<QStringRef> parts;
    QTextBoundaryFinder boundaryFinder(boundary, text);

    while (boundaryFinder.position() < text.length()) {
        const int start = boundaryFinder.position();
        const int end = boundaryFinder.toNextBoundary();
        if (end == -1) {
            break;
        }
        const int length = end - start;
        if (length < 1) {
            continue;
        }
        parts << QStringRef{&text, start, length};
    }
    return parts;
}

void SpellcheckHighlighter::autodetectLanguage(const QString &sentence)
{
    const auto lang = mLanguageGuesser->identify(sentence, mSpellchecker->availableLanguages());
    if (lang.isEmpty()) {
        return;
    }
    mSpellchecker->setLanguage(lang);
}

static bool isSpellcheckable(const QStringRef &token)
{
    if (token.isNull() || token.isEmpty()) {
        return false;
    }
    if (!token.at(0).isLetter()) {
        return false;
    }
    //TODO ignore urls and uppercase?
    return true;
}

void SpellcheckHighlighter::highlightBlock(const QString &text)
{
    //Avoid spellchecking quotes
    if (text.isEmpty() || text.at(0) == QChar{'>'}) {
        setFormat(0, text.length(), mQuoteFormat);
        return;
    }
    for (const auto &sentenceRef : split(QTextBoundaryFinder::Sentence, text)) {
        //Avoid spellchecking quotes
        if (sentenceRef.isEmpty() || sentenceRef.at(0) == QChar{'>'}) {
            continue;
        }

        const auto sentence = QString::fromRawData(sentenceRef.data(), sentenceRef.length());

        autodetectLanguage(sentence);

        const int offset = sentenceRef.position();
        for (const auto &wordRef : split(QTextBoundaryFinder::Word, sentence)) {
            //Avoid spellchecking words in progress
            //FIXME this will also prevent spellchecking a single word on a line.
            if (offset + wordRef.position() + wordRef.length() >= text.length()) {
                continue;
            }
            if (isSpellcheckable(wordRef)) {
                const auto word = QString::fromRawData(wordRef.data(), wordRef.length());
                const auto format = mSpellchecker->isMisspelled(word) ? mErrorFormat : QTextCharFormat{};
                setFormat(offset + wordRef.position(), wordRef.length(), format);
            }
        }
    }
}
