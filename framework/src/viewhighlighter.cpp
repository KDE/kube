/*
    Copyright (c) 2018 Christian Mollekopf <mollekopf@kolabsys.com>

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
#include "viewhighlighter.h"

#include <QQuickTextDocument>
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QString>

#include "syntaxhighlighter.h"

class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    using QSyntaxHighlighter::QSyntaxHighlighter;

    void setSearchString(const QString &s)
    {
        mSearchString = s;
        rehighlight();
    }

protected:
    void highlightBlock(const QString &text) override
    {
        highlightQuotes(text);

        if (!mSearchString.isEmpty()) {
            QTextCharFormat format;
            format.setFontWeight(QFont::Bold);
            format.setBackground(QColor{"#f67400"});

            QRegularExpression expression(mSearchString, QRegularExpression::CaseInsensitiveOption);
            auto i = expression.globalMatch(text);
            while (i.hasNext()) {
                auto match = i.next();
                setFormat(match.capturedStart(), match.capturedLength(), format);
            }
        }

    }

private:
    void highlightQuotes(const QString &text)
    {
        static auto quoteFormat = [] {
            QTextCharFormat quoteFormat;
            quoteFormat.setForeground(QColor{"#7f8c8d"});
            return quoteFormat;
        }();
        for (const auto &part : split(QTextBoundaryFinder::Line, text)) {
            if (!part.isEmpty() && part.at(0) == QChar{'>'}) {
                setFormat(part.position(), part.length(), quoteFormat);
            }
        }
    }

    QString mSearchString;
};

struct ViewHighlighter::Private {
    Highlighter *searchHighligher;
};


ViewHighlighter::ViewHighlighter(QObject *parent)
    : QObject(parent),
    d{new Private}
{

}

void ViewHighlighter::setTextDocument(QQuickTextDocument *document)
{
    if (document) {
        d->searchHighligher = new Highlighter{document->textDocument()};
    }
}

void ViewHighlighter::setSearchString(const QString &s)
{
    if (d->searchHighligher) {
        d->searchHighligher->setSearchString(s);
    }
}

#include "viewhighlighter.moc"
