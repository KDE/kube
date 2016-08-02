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
#include "interface_p.h"

#include <QTest>

QByteArray readMailFromFile(const QString &mailFile)
{
    QFile file(QLatin1String(MAIL_DATA_DIR) + QLatin1Char('/') + mailFile);
    file.open(QIODevice::ReadOnly);
    Q_ASSERT(file.isOpen());
    return file.readAll();
}

QByteArray join(QVector<QByteArray> vec, QByteArray sep)
{
    QByteArray ret;
    bool bInit = true;
    foreach(const auto &entry, vec) {
        if (!bInit) {
            ret += sep;
        }
        bInit = false;
        ret += entry;
    }
    return ret;
}

class InterfaceTest : public QObject
{
    Q_OBJECT
private:
    void printTree(const Part::Ptr &start, QString pre)
    {
        foreach (const auto &part, start->subParts()) {
            qWarning() << QStringLiteral("%1* %2(%3)")
                            .arg(pre)
                            .arg(QString::fromLatin1(part->type()))
                            .arg(QString::fromLatin1(join(part->availableContents(),", ")));
            printTree(part,pre + QStringLiteral("  "));
        }
    }

private slots:

    void testTextMail()
    {
        Parser parser(readMailFromFile("plaintext.mbox"));
        printTree(parser.d->mTree,QString());
        auto contentPartList = parser.collectContentParts();
        QCOMPARE(contentPartList.size(), 1);
        auto contentPart = contentPartList[0];
        QVERIFY((bool)contentPart);
        QCOMPARE(contentPart->availableContents(), QVector<QByteArray>() << "plaintext");
        auto contentList = contentPart->content("plaintext");
        QCOMPARE(contentList.size(), 1);
        QCOMPARE(contentList[0]->content(), QStringLiteral("If you can see this text it means that your email client couldn't display our newsletter properly.\nPlease visit this link to view the newsletter on our website: http://www.gog.com/newsletter/").toLocal8Bit());
        QCOMPARE(contentList[0]->charset(), QStringLiteral("utf-8").toLocal8Bit());
        QCOMPARE(contentList[0]->encryptions().size(), 0);
        QCOMPARE(contentList[0]->signatures().size(), 0);

        contentList = contentPart->content("html");
        QCOMPARE(contentList.size(), 0);
        auto contentAttachmentList = parser.collectAttachmentParts();
        QCOMPARE(contentAttachmentList.size(), 0);
    }

    void testTextAlternative()
    {
        Parser parser(readMailFromFile("alternative.mbox"));
        printTree(parser.d->mTree,QString());
        auto contentPartList = parser.collectContentParts();
        QCOMPARE(contentPartList.size(), 1);
        auto contentPart = contentPartList[0];
        QVERIFY((bool)contentPart);
        QCOMPARE(contentPart->availableContents(), QVector<QByteArray>() << "html" << "plaintext");
        auto contentList = contentPart->content("plaintext");
        QCOMPARE(contentList.size(), 1);
        QCOMPARE(contentList[0]->content(), QStringLiteral("If you can see this text it means that your email client couldn't display our newsletter properly.\nPlease visit this link to view the newsletter on our website: http://www.gog.com/newsletter/\n").toLocal8Bit());
        QCOMPARE(contentList[0]->charset(), QStringLiteral("utf-8").toLocal8Bit());
        QCOMPARE(contentList[0]->encryptions().size(), 0);
        QCOMPARE(contentList[0]->signatures().size(), 0);

        contentList = contentPart->content("html");
        QCOMPARE(contentList.size(), 1);
        QCOMPARE(contentList[0]->content(), QStringLiteral("<html><body><p><span>HTML</span> text</p></body></html>\n\n").toLocal8Bit());
        QCOMPARE(contentList[0]->charset(), QStringLiteral("utf-8").toLocal8Bit());
        QCOMPARE(contentList[0]->encryptions().size(), 0);
        QCOMPARE(contentList[0]->signatures().size(), 0);
        auto contentAttachmentList = parser.collectAttachmentParts();
        QCOMPARE(contentAttachmentList.size(), 0);
    }

     void testTextHtml()
    {
        Parser parser(readMailFromFile("html.mbox"));
        printTree(parser.d->mTree,QString());
        auto contentPartList = parser.collectContentParts();
        QCOMPARE(contentPartList.size(), 1);
        auto contentPart = contentPartList[0];
        QVERIFY((bool)contentPart);
        QCOMPARE(contentPart->availableContents(),  QVector<QByteArray>() << "html");

        auto contentList = contentPart->content("plaintext");
        QCOMPARE(contentList.size(), 0);

        contentList = contentPart->content("html");
        QCOMPARE(contentList.size(), 1);
        QCOMPARE(contentList[0]->content(), QStringLiteral("<html><body><p><span>HTML</span> text</p></body></html>").toLocal8Bit());
        QCOMPARE(contentList[0]->charset(), QStringLiteral("utf-8").toLocal8Bit());
        QCOMPARE(contentList[0]->encryptions().size(), 0);
        QCOMPARE(contentList[0]->signatures().size(), 0);
        auto contentAttachmentList = parser.collectAttachmentParts();
        QCOMPARE(contentAttachmentList.size(), 0);
    }

    void testSMimeEncrypted()
    {
        Parser parser(readMailFromFile("smime-encrypted.mbox"));
        printTree(parser.d->mTree,QString());
        auto contentPartList = parser.collectContentParts();
        QCOMPARE(contentPartList.size(), 1);
        auto contentPart = contentPartList[0];
        QVERIFY((bool)contentPart);
        QCOMPARE(contentPart->availableContents(),  QVector<QByteArray>() << "plaintext");
        auto contentList = contentPart->content("plaintext");
        QCOMPARE(contentList.size(), 1);
        QCOMPARE(contentList[0]->content(), QStringLiteral("The quick brown fox jumped over the lazy dog.").toLocal8Bit());
        QCOMPARE(contentList[0]->charset(), QStringLiteral("utf-8").toLocal8Bit());
        auto contentAttachmentList = parser.collectAttachmentParts();
        QCOMPARE(contentAttachmentList.size(), 0);
    }

    void testOpenPGPEncryptedAttachment()
    {
        Parser parser(readMailFromFile("openpgp-encrypted-attachment-and-non-encrypted-attachment.mbox"));
        printTree(parser.d->mTree,QString());
        auto contentPartList = parser.collectContentParts();
        QCOMPARE(contentPartList.size(), 1);
        auto contentPart = contentPartList[0];
        QVERIFY((bool)contentPart);
        QCOMPARE(contentPart->availableContents(),  QVector<QByteArray>() << "plaintext");
        auto contentList = contentPart->content("plaintext");
        QCOMPARE(contentList.size(), 1);
        QCOMPARE(contentList[0]->content(), QStringLiteral("test text").toLocal8Bit());
        QCOMPARE(contentList[0]->charset(), QStringLiteral("utf-8").toLocal8Bit());
        auto contentAttachmentList = parser.collectAttachmentParts();
        QCOMPARE(contentAttachmentList.size(), 2);
        QCOMPARE(contentAttachmentList[0]->availableContents(), QVector<QByteArray>() << "text/plain");
        QCOMPARE(contentAttachmentList[0]->content().size(), 1);
        QCOMPARE(contentAttachmentList[1]->availableContents(), QVector<QByteArray>() << "image/png");
        QCOMPARE(contentAttachmentList[1]->content().size(), 1);
    }

    void testOpenPPGInline()
    {
        Parser parser(readMailFromFile("openpgp-inline-charset-encrypted.mbox"));
        printTree(parser.d->mTree,QString());
        auto contentPartList = parser.collectContentParts();
        QCOMPARE(contentPartList.size(), 1);
        auto contentPart = contentPartList[0];
        QVERIFY((bool)contentPart);
        QCOMPARE(contentPart->availableContents(),  QVector<QByteArray>() << "plaintext");
        auto contentList = contentPart->content("plaintext");
        QCOMPARE(contentList.size(), 1);
        QCOMPARE(contentList[0]->content(), QStringLiteral("asdasd asd asd asdf sadf sdaf sadf äöü").toLocal8Bit());
        QCOMPARE(contentList[0]->charset(), QStringLiteral("utf-8").toLocal8Bit());
        auto contentAttachmentList = parser.collectAttachmentParts();
        QCOMPARE(contentAttachmentList.size(), 0);
    }
};

QTEST_GUILESS_MAIN(InterfaceTest)
#include "interfacetest.moc"