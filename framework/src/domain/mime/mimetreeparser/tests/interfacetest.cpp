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
#include <otp/objecttreeparser.h>
#include <otp/messagepart.h>

#include <QTest>
#include <QDebug>

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
        const auto expectedText = QStringLiteral("If you can see this text it means that your email client couldn't display our newsletter properly.\nPlease visit this link to view the newsletter on our website: http://www.gog.com/newsletter/");
        MimeTreeParser::ObjectTreeParser otp;
        otp.parseObjectTree(readMailFromFile("plaintext.mbox"));
        auto partList = otp.collectContentParts();
        QCOMPARE(partList.size(), 1);
        auto part = partList[0].dynamicCast<MimeTreeParser::MessagePart>();
        QCOMPARE(part->text(), expectedText);
        QCOMPARE(part->charset(), QStringLiteral("utf-8").toLocal8Bit());
        //
        // QCOMPARE(contentList[0]->encryptions().size(), 0);
        // QCOMPARE(contentList[0]->signatures().size(), 0);

        // contentList = contentPart->content("html");
        // QCOMPARE(contentList.size(), 0);
        // auto contentAttachmentList = parser.collectAttachmentParts();
        // QCOMPARE(contentAttachmentList.size(), 0);

        QCOMPARE(otp.plainTextContent(), expectedText);
        QVERIFY(otp.htmlContent().isEmpty());
    }

    void testAlternative()
    {
        MimeTreeParser::ObjectTreeParser otp;
        otp.parseObjectTree(readMailFromFile("alternative.mbox"));
        auto partList = otp.collectContentParts();
        QCOMPARE(partList.size(), 1);
        auto part = partList[0].dynamicCast<MimeTreeParser::AlternativeMessagePart>();
        QVERIFY(bool(part));
        QCOMPARE(part->plaintextContent(), QStringLiteral("If you can see this text it means that your email client couldn't display our newsletter properly.\nPlease visit this link to view the newsletter on our website: http://www.gog.com/newsletter/\n"));
        //FIXME html charset is different from plain, and both are not ISO-8859-1
        QCOMPARE(part->charset(), QStringLiteral("ISO-8859-1").toLocal8Bit());
        QCOMPARE(part->htmlContent(), QStringLiteral("<html><body><p><span>HTML</span> text</p></body></html>\n\n"));
        auto contentAttachmentList = otp.collectAttachmentParts();
        QCOMPARE(contentAttachmentList.size(), 0);
    //     QCOMPARE(contentList[0]->encryptions().size(), 0);
    //     QCOMPARE(contentList[0]->signatures().size(), 0);
    }

    void testTextHtml()
    {
        auto expectedText = QStringLiteral("<html><body><p><span>HTML</span> text</p></body></html>");
        MimeTreeParser::ObjectTreeParser otp;
        otp.parseObjectTree(readMailFromFile("html.mbox"));
        otp.print();
        auto partList = otp.collectContentParts();
        QCOMPARE(partList.size(), 1);
        auto part = partList[0].dynamicCast<MimeTreeParser::HtmlMessagePart>();
        QVERIFY(bool(part));
        QCOMPARE(part->htmlContent(), expectedText);
        QCOMPARE(part->charset(), QStringLiteral("windows-1252").toLocal8Bit());
    //     QCOMPARE(contentList[0]->encryptions().size(), 0);
    //     QCOMPARE(contentList[0]->signatures().size(), 0);
        auto contentAttachmentList = otp.collectAttachmentParts();
        QCOMPARE(contentAttachmentList.size(), 0);
        QCOMPARE(otp.htmlContent(), expectedText);
        QVERIFY(otp.plainTextContent().isEmpty());
    }

    void testSMimeEncrypted()
    {
        MimeTreeParser::ObjectTreeParser otp;
        otp.parseObjectTree(readMailFromFile("smime-encrypted.mbox"));
        otp.print();
        auto partList = otp.collectContentParts();
        QCOMPARE(partList.size(), 1);
        auto part = partList[0].dynamicCast<MimeTreeParser::MessagePart>();
        QVERIFY(bool(part));
        QCOMPARE(part->text(), QStringLiteral("The quick brown fox jumped over the lazy dog."));
        QCOMPARE(part->charset(), QStringLiteral("us-ascii").toLocal8Bit());
    //     QCOMPARE(contentList[0]->encryptions().size(), 1);
    //     QCOMPARE(contentList[0]->signatures().size(), 0);
    //     auto contentAttachmentList = parser.collectAttachmentParts();
    //     QCOMPARE(contentAttachmentList.size(), 0);
    }

    void testOpenPGPEncryptedAttachment()
    {
        MimeTreeParser::ObjectTreeParser otp;
        otp.parseObjectTree(readMailFromFile("openpgp-encrypted-attachment-and-non-encrypted-attachment.mbox"));
        otp.print();
        auto partList = otp.collectContentParts();
        QCOMPARE(partList.size(), 1);
        auto part = partList[0].dynamicCast<MimeTreeParser::MessagePart>();
        QVERIFY(bool(part));
        QCOMPARE(part->text(), QStringLiteral("test text"));
        QCOMPARE(part->charset(), QStringLiteral("us-ascii").toLocal8Bit());
    //     QCOMPARE(contentList[0]->encryptions().size(), 1);
    //     QCOMPARE(contentList[0]->signatures().size(), 1);
        auto contentAttachmentList = otp.collectAttachmentParts();
        QCOMPARE(contentAttachmentList.size(), 2);
    //     QCOMPARE(contentAttachmentList[0]->availableContents(), QVector<QByteArray>() << "text/plain");
    //     QCOMPARE(contentAttachmentList[0]->content().size(), 1);
    //     QCOMPARE(contentAttachmentList[0]->encryptions().size(), 1);
    //     QCOMPARE(contentAttachmentList[0]->signatures().size(), 1);
    //     QCOMPARE(contentAttachmentList[1]->availableContents(), QVector<QByteArray>() << "image/png");
    //     QCOMPARE(contentAttachmentList[1]->content().size(), 1);
    //     QCOMPARE(contentAttachmentList[1]->encryptions().size(), 0);
    //     QCOMPARE(contentAttachmentList[1]->signatures().size(), 0);
    }

    void testOpenPGPInline()
    {
        MimeTreeParser::ObjectTreeParser otp;
        otp.parseObjectTree(readMailFromFile("openpgp-inline-charset-encrypted.mbox"));
        otp.print();
        auto partList = otp.collectContentParts();
        QCOMPARE(partList.size(), 1);
        auto part = partList[0].dynamicCast<MimeTreeParser::MessagePart>();
        QVERIFY(bool(part));
        QEXPECT_FAIL("", "Somethings wrong with the encoding", Continue);
        // qWarning() << part->text();
        QCOMPARE(part->text(), QString::fromUtf8("asdasd asd asd asdf sadf sdaf sadf äöü"));
        QCOMPARE(part->charset(), QStringLiteral("ISO-8859-15").toLocal8Bit());

        // QCOMPARE(contentList[0]->encryptions().size(), 1);
        // QCOMPARE(contentList[0]->signatures().size(), 1);
        // auto contentAttachmentList = parser.collectAttachmentParts();
        // QCOMPARE(contentAttachmentList.size(), 0);
    }

    void testOpenPPGInlineWithNonEncText()
    {
        MimeTreeParser::ObjectTreeParser otp;
        otp.parseObjectTree(readMailFromFile("openpgp-inline-encrypted+nonenc.mbox"));
        otp.print();
        auto partList = otp.collectContentParts();
        QCOMPARE(partList.size(), 1);
        auto part1 = partList[0].dynamicCast<MimeTreeParser::MessagePart>();
        QVERIFY(bool(part1));
        QCOMPARE(part1->text(), QStringLiteral("Not encrypted not signed :(\n\nsome random text"));
        //TODO test if we get the proper subparts with the appropriate encryptions
        QCOMPARE(part1->charset(), QStringLiteral("us-ascii").toLocal8Bit());

        // QCOMPARE(part1->text(), QStringLiteral("Not encrypted not signed :(\n\n"));
        // QCOMPARE(part1->charset(), QStringLiteral("us-ascii").toLocal8Bit());
        // QCOMPARE(contentList[1]->content(), QStringLiteral("some random text").toLocal8Bit());
        // QCOMPARE(contentList[1]->charset(), QStringLiteral("us-ascii").toLocal8Bit());
        // QCOMPARE(contentList[1]->encryptions().size(), 1);
        // QCOMPARE(contentList[1]->signatures().size(), 0);
        auto contentAttachmentList = otp.collectAttachmentParts();
        QCOMPARE(contentAttachmentList.size(), 0);
    }

    void testEncryptionBlock()
    {
        MimeTreeParser::ObjectTreeParser otp;
        otp.parseObjectTree(readMailFromFile("openpgp-encrypted-attachment-and-non-encrypted-attachment.mbox"));
        otp.print();
        auto partList = otp.collectContentParts();
        QCOMPARE(partList.size(), 1);
        auto part1 = partList[0].dynamicCast<MimeTreeParser::MessagePart>();
        QVERIFY(bool(part1));
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
        auto attachmentList = otp.collectAttachmentParts();
        QCOMPARE(attachmentList.size(), 2);
        auto attachment1 = attachmentList[0];
        QVERIFY(attachment1->node());
        QCOMPARE(attachment1->filename(), QStringLiteral("file.txt"));
        auto attachment2 = attachmentList[1];
        QVERIFY(attachment2->node());
        QCOMPARE(attachment2->filename(), QStringLiteral("image.png"));
    }

    void testSignatureBlock()
    {
        MimeTreeParser::ObjectTreeParser otp;
        otp.parseObjectTree(readMailFromFile("openpgp-encrypted-attachment-and-non-encrypted-attachment.mbox"));
        otp.print();
        auto partList = otp.collectContentParts();
        QCOMPARE(partList.size(), 1);
        auto part = partList[0].dynamicCast<MimeTreeParser::MessagePart>();
        QVERIFY(bool(part));

        // QCOMPARE(contentList[0]->signatures().size(), 1);
        // auto sig = contentList[0]->signatures()[0];
        // QCOMPARE(sig->creationDateTime(), QDateTime(QDate(2015,05,01),QTime(15,12,47)));
        // QCOMPARE(sig->expirationDateTime(), QDateTime());
        // QCOMPARE(sig->neverExpires(), true);

        // auto key = sig->key();
        // QCOMPARE(key->keyid(),QStringLiteral("8D9860C58F246DE6"));
        // QCOMPARE(key->name(),QStringLiteral("unittest key"));
        // QCOMPARE(key->email(),QStringLiteral("test@kolab.org"));
        // QCOMPARE(key->comment(),QStringLiteral("no password"));
    }

    void testRelatedAlternative()
    {
        MimeTreeParser::ObjectTreeParser otp;
        otp.parseObjectTree(readMailFromFile("cid-links.mbox"));
        otp.print();
        auto partList = otp.collectContentParts();
        QCOMPARE(partList.size(), 1);
        auto part = partList[0].dynamicCast<MimeTreeParser::MessagePart>();
        QVERIFY(bool(part));
    //     QCOMPARE(contentPart->availableContents(),  QVector<QByteArray>() <<  "html" << "plaintext");
    //     QCOMPARE(contentPart->encryptions().size(), 0);
    //     QCOMPARE(contentPart->signatures().size(), 0);
    //     auto contentList = contentPart->content("plaintext");
    //     QCOMPARE(contentList.size(), 1);
    //     auto contentAttachmentList = parser.collectAttachmentParts();
    //     QCOMPARE(contentAttachmentList.size(), 0);
    }

    void testAttachmentPart()
    {
        MimeTreeParser::ObjectTreeParser otp;
        otp.parseObjectTree(readMailFromFile("cid-links.mbox"));
        otp.print();
        auto partList = otp.collectAttachmentParts();
        QCOMPARE(partList.size(), 1);
        auto part = partList[0].dynamicCast<MimeTreeParser::MessagePart>();
        QVERIFY(bool(part));
        auto att = part->node();
        qWarning() << "Attachment type: " << att->contentType(true)->mimeType();
        QCOMPARE(part->mimeType(), QByteArray("image/jpeg"));

        // Parser parser(readMailFromFile("cid-links.mbox"));
        // const auto relatedImage = parser.d->mTree->subParts().at(1);
        // QCOMPARE(relatedImage->availableContents(),  QVector<QByteArray>() <<  "image/jpeg");
        // auto contentList = relatedImage->content();
        // QCOMPARE(contentList.size(), 1);
        // contentList = relatedImage->content("image/jpeg");
        // QCOMPARE(contentList.size(), 1);
    }

    void testCidLink()
    {
        MimeTreeParser::ObjectTreeParser otp;
        otp.parseObjectTree(readMailFromFile("cid-links.mbox"));
        otp.print();
        auto partList = otp.collectContentParts();
        QCOMPARE(partList.size(), 1);
        auto part = partList[0].dynamicCast<MimeTreeParser::AlternativeMessagePart>();
        QVERIFY(bool(part));
        auto resolvedContent = otp.resolveCidLinks(part->htmlContent());
        QVERIFY(!resolvedContent.contains("cid:"));
    }

    void testCidLinkInForwardedInline()
    {
        MimeTreeParser::ObjectTreeParser otp;
        otp.parseObjectTree(readMailFromFile("cid-links-forwarded-inline.mbox"));
        otp.print();
        auto partList = otp.collectContentParts();
        QCOMPARE(partList.size(), 1);
        auto part = partList[0].dynamicCast<MimeTreeParser::AlternativeMessagePart>();
        QVERIFY(bool(part));
        auto resolvedContent = otp.resolveCidLinks(part->htmlContent());
        QVERIFY(!resolvedContent.contains("cid:"));
    }
};

QTEST_GUILESS_MAIN(InterfaceTest)
#include "interfacetest.moc"
