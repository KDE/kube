/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "converthtmltoplaintext.h"

#include <grantlee/plaintextmarkupbuilder.h>
#include <QTextDocument>
using namespace MessageViewer;
ConvertHtmlToPlainText::ConvertHtmlToPlainText()
{

}

ConvertHtmlToPlainText::~ConvertHtmlToPlainText()
{

}

void ConvertHtmlToPlainText::setHtmlString(const QString &htmlString)
{
    mHtmlString = htmlString;
}

QString ConvertHtmlToPlainText::generatePlainText()
{
    if (mHtmlString.isEmpty()) {
        return QString();
    }
    Grantlee::PlainTextMarkupBuilder *pb = new Grantlee::PlainTextMarkupBuilder();

    Grantlee::MarkupDirector *pmd = new Grantlee::MarkupDirector(pb);
    QTextDocument *doc = new QTextDocument;
    doc->setHtml(mHtmlString);

    pmd->processDocument(doc);
    QString plainText = pb->getResult();

    delete doc;
    delete pmd;
    delete pb;
    toCleanPlainText(plainText);
    return plainText;
}

QString ConvertHtmlToPlainText::htmlString() const
{
    return mHtmlString;
}

//Duplicate from kpimtextedit/textedit.h
void ConvertHtmlToPlainText::toCleanPlainText(QString &text)
{
    // Remove line separators. Normal \n chars are still there, so no linebreaks get lost here
    text.remove(QChar::LineSeparator);

    // Get rid of embedded images, see QTextImageFormat documentation:
    // "Inline images are represented by an object replacement character (0xFFFC in Unicode) "
    text.remove(0xFFFC);

    // In plaintext mode, each space is non-breaking.
    text.replace(QChar::Nbsp, QChar::fromLatin1(' '));
}
