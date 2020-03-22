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
#include <Sonnet/GuessLanguage>

SpellcheckHighlighter::SpellcheckHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent),
    mSpellchecker{new Sonnet::Speller()}
{
    mErrorFormat.setForeground(Qt::red);

    if (!mSpellchecker->isValid()) {
        qWarning() << "Spellchecker is invalid";
    }
    qDebug() << mSpellchecker->availableDictionaries();

}

struct Position {
    int start, length;
};

static QList<Position> sentences(const QString &text)
{
    QList<Position> breaks;
    QTextBoundaryFinder boundaryFinder(QTextBoundaryFinder::Sentence, text);

    while (boundaryFinder.position() < text.length()) {
        Position pos;
        pos.start = boundaryFinder.position();
        int end = boundaryFinder.toNextBoundary();
        if (end == -1) {
            break;
        }
        pos.length = end - pos.start;
        if (pos.length < 1) {
            continue;
        }
        breaks.append(pos);
    }
    return breaks;
}

static QList<Position> words(const QString &text)
{
    QList<Position> breaks;
    QTextBoundaryFinder boundaryFinder(QTextBoundaryFinder::Word, text);

    while (boundaryFinder.position() < text.length()) {
        if (!(boundaryFinder.boundaryReasons().testFlag(QTextBoundaryFinder::StartOfItem))) {
            if (boundaryFinder.toNextBoundary() == -1) {
                break;
            }
            continue;
        }

        Position pos;
        pos.start = boundaryFinder.position();
        int end = boundaryFinder.toNextBoundary();
        if (end == -1) {
            break;
        }
        pos.length = end - pos.start;
        if (pos.length < 1) {
            continue;
        }
        breaks.append(pos);

        if (boundaryFinder.toNextBoundary() == -1) {
            break;
        }
    }
    return breaks;
}

void SpellcheckHighlighter::autodetectLanguage(const QStringRef &sentence)
{
    Sonnet::GuessLanguage gl;
    const auto lang = gl.identify(sentence.toString(), mSpellchecker->availableLanguages());
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
    for (const auto &pos : sentences(text)) {
        const QStringRef sentence(&text, pos.start, pos.length);

        autodetectLanguage(sentence);

        const int offset = sentence.position();
        for (const auto &pos : words(text)) {
            const QStringRef word(&text, pos.start, pos.length);
            if (isSpellcheckable(word)) {
                setFormat(word.position() + offset, word.length(), mSpellchecker->isMisspelled(word.toString()) ? mErrorFormat : QTextCharFormat{});
            }
        }
    }
    setCurrentBlockState(0);
}
