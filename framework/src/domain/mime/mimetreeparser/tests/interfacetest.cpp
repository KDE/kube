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

#include <otp/objecttreeparser.h>
#include <otp/messagepart.h>

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
        MimeTreeParser::ObjectTreeParser otp;
        otp.parseObjectTree(readMailFromFile("plaintext.mbox"));
        auto partList = otp.collectContentParts();
        QCOMPARE(partList.size(), 1);
        auto part = partList[0].dynamicCast<MimeTreeParser::MessagePart>();
        QCOMPARE(part->text(), QStringLiteral("If you can see this text it means that your email client couldn't display our newsletter properly.\nPlease visit this link to view the newsletter on our website: http://www.gog.com/newsletter/"));
        //TODO
        // QCOMPARE(part->charset(), QStringLiteral("utf-8").toLocal8Bit());
        //
        // QCOMPARE(contentList[0]->encryptions().size(), 0);
        // QCOMPARE(contentList[0]->signatures().size(), 0);

        // contentList = contentPart->content("html");
        // QCOMPARE(contentList.size(), 0);
        // auto contentAttachmentList = parser.collectAttachmentParts();
        // QCOMPARE(contentAttachmentList.size(), 0);
    }

    // {
    //     Parser parser(readMailFromFile("alternative.mbox"));
    //     printTree(parser.d->mTree,QString());
    //     auto contentPartList = parser.collectContentParts();
    //     QCOMPARE(contentPartList.size(), 1);
    //     auto contentPart = contentPartList[0];
    //     QVERIFY((bool)contentPart);
    //     QCOMPARE(contentPart->availableContents(), QVector<QByteArray>() << "html" << "plaintext");
    //     auto contentList = contentPart->content("plaintext");
    //     QCOMPARE(contentList.size(), 1);
    //     QCOMPARE(contentList[0]->content(), QStringLiteral("If you can see this text it means that your email client couldn't display our newsletter properly.\nPlease visit this link to view the newsletter on our website: http://www.gog.com/newsletter/\n").toLocal8Bit());
    //     QCOMPARE(contentList[0]->charset(), QStringLiteral("utf-8").toLocal8Bit());
    //     QCOMPARE(contentList[0]->encryptions().size(), 0);
    //     QCOMPARE(contentList[0]->signatures().size(), 0);

    //     contentList = contentPart->content("html");
    //     QCOMPARE(contentList.size(), 1);
    //     QCOMPARE(contentList[0]->content(), QStringLiteral("<html><body><p><span>HTML</span> text</p></body></html>\n\n").toLocal8Bit());
    //     QCOMPARE(contentList[0]->charset(), QStringLiteral("utf-8").toLocal8Bit());
    //     QCOMPARE(contentList[0]->encryptions().size(), 0);
    //     QCOMPARE(contentList[0]->signatures().size(), 0);
    //     auto contentAttachmentList = parser.collectAttachmentParts();
    //     QCOMPARE(contentAttachmentList.size(), 0);
    // }
    
    // void testAlternative()
    // {
    //     Parser parser(readMailFromFile("alternative.mbox"));
    //     printTree(parser.d->mTree,QString());
    //     auto contentPartList = parser.collectContentParts();
    //     QCOMPARE(contentPartList.size(), 1);
    //     auto contentPart = contentPartList[0];
    //     QVERIFY((bool)contentPart);
    //     QCOMPARE(contentPart->availableContents(), QVector<QByteArray>() << "html" << "plaintext");
    //     auto contentList = contentPart->content("plaintext");
    //     QCOMPARE(contentList.size(), 1);
    //     QCOMPARE(contentList[0]->content(), QStringLiteral("If you can see this text it means that your email client couldn't display our newsletter properly.\nPlease visit this link to view the newsletter on our website: http://www.gog.com/newsletter/\n").toLocal8Bit());
    //     QCOMPARE(contentList[0]->charset(), QStringLiteral("utf-8").toLocal8Bit());
    //     QCOMPARE(contentList[0]->encryptions().size(), 0);
    //     QCOMPARE(contentList[0]->signatures().size(), 0);

    //     contentList = contentPart->content("html");
    //     QCOMPARE(contentList.size(), 1);
    //     QCOMPARE(contentList[0]->content(), QStringLiteral("<html><body><p><span>HTML</span> text</p></body></html>\n\n").toLocal8Bit());
    //     QCOMPARE(contentList[0]->charset(), QStringLiteral("utf-8").toLocal8Bit());
    //     QCOMPARE(contentList[0]->encryptions().size(), 0);
    //     QCOMPARE(contentList[0]->signatures().size(), 0);
    //     auto contentAttachmentList = parser.collectAttachmentParts();
    //     QCOMPARE(contentAttachmentList.size(), 0);
    // }

    //  void testTextHtml()
    // {
    //     Parser parser(readMailFromFile("html.mbox"));
    //     printTree(parser.d->mTree,QString());
    //     auto contentPartList = parser.collectContentParts();
    //     QCOMPARE(contentPartList.size(), 1);
    //     auto contentPart = contentPartList[0];
    //     QVERIFY((bool)contentPart);
    //     QCOMPARE(contentPart->availableContents(),  QVector<QByteArray>() << "html");

    //     auto contentList = contentPart->content("plaintext");
    //     QCOMPARE(contentList.size(), 0);

    //     contentList = contentPart->content("html");
    //     QCOMPARE(contentList.size(), 1);
    //     QCOMPARE(contentList[0]->content(), QStringLiteral("<html><body><p><span>HTML</span> text</p></body></html>").toLocal8Bit());
    //     QCOMPARE(contentList[0]->charset(), QStringLiteral("utf-8").toLocal8Bit());
    //     QCOMPARE(contentList[0]->encryptions().size(), 0);
    //     QCOMPARE(contentList[0]->signatures().size(), 0);
    //     auto contentAttachmentList = parser.collectAttachmentParts();
    //     QCOMPARE(contentAttachmentList.size(), 0);
    // }

    // void testSMimeEncrypted()
    // {
    //     Parser parser(readMailFromFile("smime-encrypted.mbox"));
    //     printTree(parser.d->mTree,QString());
    //     auto contentPartList = parser.collectContentParts();
    //     QCOMPARE(contentPartList.size(), 1);
    //     auto contentPart = contentPartList[0];
    //     QVERIFY((bool)contentPart);
    //     QCOMPARE(contentPart->availableContents(),  QVector<QByteArray>() << "plaintext");
    //     auto contentList = contentPart->content("plaintext");
    //     QCOMPARE(contentList.size(), 1);
    //     QCOMPARE(contentList[0]->content(), QStringLiteral("The quick brown fox jumped over the lazy dog.").toLocal8Bit());
    //     QCOMPARE(contentList[0]->charset(), QStringLiteral("us-ascii").toLocal8Bit());
    //     QCOMPARE(contentList[0]->encryptions().size(), 1);
    //     QCOMPARE(contentList[0]->signatures().size(), 0);
    //     auto contentAttachmentList = parser.collectAttachmentParts();
    //     QCOMPARE(contentAttachmentList.size(), 0);
    // }

    // void testOpenPGPEncryptedAttachment()
    // {
    //     Parser parser(readMailFromFile("openpgp-encrypted-attachment-and-non-encrypted-attachment.mbox"));
    //     printTree(parser.d->mTree,QString());
    //     auto contentPartList = parser.collectContentParts();
    //     QCOMPARE(contentPartList.size(), 1);
    //     auto contentPart = contentPartList[0];
    //     QVERIFY((bool)contentPart);
    //     QCOMPARE(contentPart->availableContents(),  QVector<QByteArray>() << "plaintext");
    //     auto contentList = contentPart->content("plaintext");
    //     QCOMPARE(contentList.size(), 1);
    //     QCOMPARE(contentList[0]->content(), QStringLiteral("test text").toLocal8Bit());
    //     QCOMPARE(contentList[0]->charset(), QStringLiteral("us-ascii").toLocal8Bit());
    //     QCOMPARE(contentList[0]->encryptions().size(), 1);
    //     QCOMPARE(contentList[0]->signatures().size(), 1);
    //     auto contentAttachmentList = parser.collectAttachmentParts();
    //     QCOMPARE(contentAttachmentList.size(), 2);
    //     QCOMPARE(contentAttachmentList[0]->availableContents(), QVector<QByteArray>() << "text/plain");
    //     QCOMPARE(contentAttachmentList[0]->content().size(), 1);
    //     QCOMPARE(contentAttachmentList[0]->encryptions().size(), 1);
    //     QCOMPARE(contentAttachmentList[0]->signatures().size(), 1);
    //     QCOMPARE(contentAttachmentList[1]->availableContents(), QVector<QByteArray>() << "image/png");
    //     QCOMPARE(contentAttachmentList[1]->content().size(), 1);
    //     QCOMPARE(contentAttachmentList[1]->encryptions().size(), 0);
    //     QCOMPARE(contentAttachmentList[1]->signatures().size(), 0);
    // }

    // void testOpenPGPInline()
    // {
    //     Parser parser(readMailFromFile("openpgp-inline-charset-encrypted.mbox"));
    //     printTree(parser.d->mTree,QString());
    //     auto contentPartList = parser.collectContentParts();
    //     QCOMPARE(contentPartList.size(), 1);
    //     auto contentPart = contentPartList[0];
    //     QVERIFY((bool)contentPart);
    //     QCOMPARE(contentPart->availableContents(),  QVector<QByteArray>() << "plaintext");
    //     QCOMPARE(contentPart->encryptions().size(), 0);
    //     QCOMPARE(contentPart->signatures().size(), 0);
    //     auto contentList = contentPart->content("plaintext");
    //     QCOMPARE(contentList.size(), 1);
    //     QCOMPARE(contentList[0]->content(), QStringLiteral("asdasd asd asd asdf sadf sdaf sadf äöü").toLocal8Bit());
    //     QCOMPARE(contentList[0]->charset(), QStringLiteral("ISO-8859-15").toLocal8Bit());
    //     QCOMPARE(contentList[0]->encryptions().size(), 1);
    //     QCOMPARE(contentList[0]->signatures().size(), 1);
    //     auto contentAttachmentList = parser.collectAttachmentParts();
    //     QCOMPARE(contentAttachmentList.size(), 0);
    // }

    // void testOpenPPGInlineWithNonEncText()
    // {
    //     Parser parser(readMailFromFile("openpgp-inline-encrypted+nonenc.mbox"));
    //     printTree(parser.d->mTree,QString());
    //     auto contentPartList = parser.collectContentParts();
    //     QCOMPARE(contentPartList.size(), 1);
    //     auto contentPart = contentPartList[0];
    //     QVERIFY((bool)contentPart);
    //     QCOMPARE(contentPart->availableContents(),  QVector<QByteArray>() << "plaintext");
    //     QCOMPARE(contentPart->encryptions().size(), 0);
    //     QCOMPARE(contentPart->signatures().size(), 0);
    //     auto contentList = contentPart->content("plaintext");
    //     QCOMPARE(contentList.size(), 2);
    //     QCOMPARE(contentList[0]->content(), QStringLiteral("Not encrypted not signed :(\n\n").toLocal8Bit());
    //     QCOMPARE(contentList[0]->charset(), QStringLiteral("us-ascii").toLocal8Bit());
    //     QCOMPARE(contentList[0]->encryptions().size(), 0);
    //     QCOMPARE(contentList[0]->signatures().size(), 0);
    //     QCOMPARE(contentList[1]->content(), QStringLiteral("some random text").toLocal8Bit());
    //     QCOMPARE(contentList[1]->charset(), QStringLiteral("us-ascii").toLocal8Bit());
    //     QCOMPARE(contentList[1]->encryptions().size(), 1);
    //     QCOMPARE(contentList[1]->signatures().size(), 0);
    //     auto contentAttachmentList = parser.collectAttachmentParts();
    //     QCOMPARE(contentAttachmentList.size(), 0);
    // }

    // void testEncryptionBlock()
    // {
    //     Parser parser(readMailFromFile("openpgp-encrypted-attachment-and-non-encrypted-attachment.mbox"));
    //     auto contentPartList = parser.collectContentParts();
    //     auto contentPart = contentPartList[0];
    //     auto contentList = contentPart->content("plaintext");
    //     QCOMPARE(contentList.size(), 1);
    //     QCOMPARE(contentList[0]->encryptions().size(), 1);
    //     auto enc = contentList[0]->encryptions()[0];
    //     QCOMPARE((int) enc->recipients().size(), 2);

    //     auto r = enc->recipients()[0];
    //     QCOMPARE(r->keyid(),QStringLiteral("14B79E26050467AA"));
    //     QCOMPARE(r->name(),QStringLiteral("kdetest"));
    //     QCOMPARE(r->email(),QStringLiteral("you@you.com"));
    //     QCOMPARE(r->comment(),QStringLiteral(""));

    //     r = enc->recipients()[1];
    //     QCOMPARE(r->keyid(),QStringLiteral("8D9860C58F246DE6"));
    //     QCOMPARE(r->name(),QStringLiteral("unittest key"));
    //     QCOMPARE(r->email(),QStringLiteral("test@kolab.org"));
    //     QCOMPARE(r->comment(),QStringLiteral("no password"));
    // }

    // void testSignatureBlock()
    // {
    //     Parser parser(readMailFromFile("openpgp-encrypted-attachment-and-non-encrypted-attachment.mbox"));
    //     auto contentPartList = parser.collectContentParts();
    //     auto contentPart = contentPartList[0];
    //     auto contentList = contentPart->content("plaintext");
    //     QCOMPARE(contentList.size(), 1);
    //     QCOMPARE(contentList[0]->signatures().size(), 1);
    //     auto sig = contentList[0]->signatures()[0];
    //     QCOMPARE(sig->creationDateTime(), QDateTime(QDate(2015,05,01),QTime(15,12,47)));
    //     QCOMPARE(sig->expirationDateTime(), QDateTime());
    //     QCOMPARE(sig->neverExpires(), true);

    //     auto key = sig->key();
    //     QCOMPARE(key->keyid(),QStringLiteral("8D9860C58F246DE6"));
    //     QCOMPARE(key->name(),QStringLiteral("unittest key"));
    //     QCOMPARE(key->email(),QStringLiteral("test@kolab.org"));
    //     QCOMPARE(key->comment(),QStringLiteral("no password"));
    // }

    // void testRelatedAlternative()
    // {
    //     Parser parser(readMailFromFile("cid-links.mbox"));
    //     printTree(parser.d->mTree,QString());
    //     auto contentPartList = parser.collectContentParts();
    //     QCOMPARE(contentPartList.size(), 1);
    //     auto contentPart = contentPartList[0];
    //     QVERIFY((bool)contentPart);
    //     QCOMPARE(contentPart->availableContents(),  QVector<QByteArray>() <<  "html" << "plaintext");
    //     QCOMPARE(contentPart->encryptions().size(), 0);
    //     QCOMPARE(contentPart->signatures().size(), 0);
    //     auto contentList = contentPart->content("plaintext");
    //     QCOMPARE(contentList.size(), 1);
    //     auto contentAttachmentList = parser.collectAttachmentParts();
    //     QCOMPARE(contentAttachmentList.size(), 0);
    // }

    // void testAttachmentPart()
    // {
    //     Parser parser(readMailFromFile("cid-links.mbox"));
    //     const auto relatedImage = parser.d->mTree->subParts().at(1);
    //     QCOMPARE(relatedImage->availableContents(),  QVector<QByteArray>() <<  "image/jpeg");
    //     auto contentList = relatedImage->content();
    //     QCOMPARE(contentList.size(), 1);
    //     contentList = relatedImage->content("image/jpeg");
    //     QCOMPARE(contentList.size(), 1);
    // }

    // void testCidLink()
    // {
    //     Parser parser(readMailFromFile("cid-links.mbox"));
    //     printTree(parser.d->mTree,QString());
    //     QCOMPARE(parser.getPart(QUrl("cid:9359201d15e53f31a68c307b3369b6@info")), parser.d->mTree->subParts().at(1));
    //     QVERIFY(!parser.getPart(QUrl("cid:")));
    //     QVERIFY(!parser.getPart(QUrl("cid:unknown")));
    // }

    // void testCidLinkInForwardedInline()
    // {
    //     Parser parser(readMailFromFile("cid-links-forwarded-inline.mbox"));
    //     printTree(parser.d->mTree,QString());
    //     //Make sure we can find the attachment with the image in the forwarded message
    //     QVERIFY(bool(parser.getPart(QUrl("cid:1496058170592c093a661dd956000502@info"))));
    //     // QCOMPARE(parser.getPart(QUrl("cid:1496058170592c093a661dd956000502@info")), parser.d->mTree->subParts().at(1));
    //     QVERIFY(!parser.getPart(QUrl("cid:")));
    //     QVERIFY(!parser.getPart(QUrl("cid:unknown")));
    // }
};

QTEST_GUILESS_MAIN(InterfaceTest)
#include "interfacetest.moc"
