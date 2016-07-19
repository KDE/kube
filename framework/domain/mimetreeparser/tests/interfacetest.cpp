/*
    Copyright (c) 2016 Sandro Knauß <knauss@kolabsystems.com>

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

#include "interface.h"

#include <QTest>

QByteArray readMailFromFile(const QString &mailFile)
{
    QFile file(QLatin1String(MAIL_DATA_DIR) + QLatin1Char('/') + mailFile);
    file.open(QIODevice::ReadOnly);
    Q_ASSERT(file.isOpen());
    return file.readAll();
}


class InterfaceTest : public QObject
{
    Q_OBJECT
private slots:

    void testTextMail()
    {
        Parser parser(readMailFromFile("plaintext.mbox"));
        auto contentPart = parser.collectContentPart();
        QVERIFY((bool)contentPart);
        QCOMPARE(contentPart->availableContents(), ContentPart::PlainText);
        auto contentList = contentPart->content(ContentPart::PlainText);
        QCOMPARE(contentList.size(), 1);
        QCOMPARE(contentList[0]->content(), QStringLiteral("If you can see this text it means that your email client couldn't display our newsletter properly.\nPlease visit this link to view the newsletter on our website: http://www.gog.com/newsletter/\n\n- GOG.com Team\n\n").toLocal8Bit());
        QCOMPARE(contentList[0]->charset(), QStringLiteral("utf-8").toLocal8Bit());
        QCOMPARE(contentList[0]->encryptions().size(), 0);
        QCOMPARE(contentList[0]->signatures().size(), 0);
    }

    void testTextAlternative()
    {
        Parser parser(readMailFromFile("alternative.mbox"));
        auto contentPart = parser.collectContentPart();
        QVERIFY((bool)contentPart);
        QCOMPARE(contentPart->availableContents(), ContentPart::PlainText | ContentPart::Html);
    }

     void testTextHtml()
    {
        Parser parser(readMailFromFile("html.mbox"));
        auto contentPart = parser.collectContentPart();
        QVERIFY((bool)contentPart);
        QCOMPARE(contentPart->availableContents(), ContentPart::Html);
    }
};

QTEST_GUILESS_MAIN(InterfaceTest)
#include "interfacetest.moc"