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

class SearchHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    SearchHighlighter(QTextDocument *parent = 0)
        : QSyntaxHighlighter(parent)
    {
    }

    void setSearchString(const QString &s)
    {
        mSearchString = s;
        rehighlight();
    }

protected:
    void highlightBlock(const QString &text) override
    {
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
    QString mSearchString;
};

struct ViewHighlighter::Private {
    SearchHighlighter *searchHighligher;

};


ViewHighlighter::ViewHighlighter(QObject *parent)
    : QObject(parent),
    d{new Private}
{

}

void ViewHighlighter::setTextDocument(QQuickTextDocument *document)
{
    if (document) {
        d->searchHighligher = new SearchHighlighter{document->textDocument()};
    }
}

void ViewHighlighter::setSearchString(const QString &s)
{
    if (d->searchHighligher) {
        d->searchHighligher->setSearchString(s);
    }
}

#include "viewhighlighter.moc"
