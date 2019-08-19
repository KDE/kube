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
#include <objecttreeparser.h>

#include <QTest>
#include <QDebug>

QByteArray readMailFromFile(const QString &mailFile)
{
    QFile file(QLatin1String(MAIL_DATA_DIR) + QLatin1Char('/') + mailFile);
    file.open(QIODevice::ReadOnly);
    Q_ASSERT(file.isOpen());
    return file.readAll();
}

class MimeTreeParserTest : public QObject
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

        QCOMPARE(part->encryptions().size(), 0);
        QCOMPARE(part->signatures().size(), 0);

        QCOMPARE(otp.collectAttachmentParts().size(), 0);

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
        QCOMPARE(otp.collectAttachmentParts().size(), 0);
        QCOMPARE(part->encryptions().size(), 0);
        QCOMPARE(part->signatures().size(), 0);
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
        QCOMPARE(part->encryptions().size(), 0);
        QCOMPARE(part->signatures().size(), 0);
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
        otp.decryptParts();
        otp.print();
        auto partList = otp.collectContentParts();
        QCOMPARE(partList.size(), 1);
        auto part = partList[0].dynamicCast<MimeTreeParser::MessagePart>();
        QVERIFY(bool(part));
        QCOMPARE(part->text(), QStringLiteral("The quick brown fox jumped over the lazy dog."));
        QCOMPARE(part->charset(), QStringLiteral("us-ascii").toLocal8Bit());
        QCOMPARE(part->encryptions().size(), 1);
        QCOMPARE(part->signatures().size(), 0);
        auto contentAttachmentList = otp.collectAttachmentParts();
        QCOMPARE(contentAttachmentList.size(), 0);
    }

    void testOpenPGPEncryptedAttachment()
    {
        MimeTreeParser::ObjectTreeParser otp;
        otp.parseObjectTree(readMailFromFile("openpgp-encrypted-attachment-and-non-encrypted-attachment.mbox"));
        otp.print();
        otp.decryptParts();
        otp.print();
        auto partList = otp.collectContentParts();
        QCOMPARE(partList.size(), 1);
        auto part = partList[0].dynamicCast<MimeTreeParser::MessagePart>();
        QVERIFY(bool(part));
        QCOMPARE(part->text(), QStringLiteral("test text"));
        QCOMPARE(part->charset(), QStringLiteral("us-ascii").toLocal8Bit());
        QCOMPARE(part->encryptions().size(), 1);
        QCOMPARE(part->signatures().size(), 1);
        QCOMPARE(part->encryptionState(), MimeTreeParser::KMMsgFullyEncrypted);
        QCOMPARE(part->signatureState(), MimeTreeParser::KMMsgFullySigned);
        auto contentAttachmentList = otp.collectAttachmentParts();
        QCOMPARE(contentAttachmentList.size(), 2);
    //     QCOMPARE(contentAttachmentList[0]->availableContents(), QVector<QByteArray>() << "text/plain");
        // QCOMPARE(contentAttachmentList[0]->content().size(), 1);
        QCOMPARE(contentAttachmentList[0]->encryptions().size(), 1);
        QCOMPARE(contentAttachmentList[0]->signatures().size(), 1);
        QCOMPARE(contentAttachmentList[0]->encryptionState(), MimeTreeParser::KMMsgFullyEncrypted);
        QCOMPARE(contentAttachmentList[0]->signatureState(), MimeTreeParser::KMMsgFullySigned);
    //     QCOMPARE(contentAttachmentList[1]->availableContents(), QVector<QByteArray>() << "image/png");
    //     QCOMPARE(contentAttachmentList[1]->content().size(), 1);
        QCOMPARE(contentAttachmentList[1]->encryptions().size(), 0);
        QCOMPARE(contentAttachmentList[1]->signatures().size(), 0);
        QCOMPARE(contentAttachmentList[1]->encryptionState(), MimeTreeParser::KMMsgNotEncrypted);
        QCOMPARE(contentAttachmentList[1]->signatureState(), MimeTreeParser::KMMsgNotSigned);
    }

    void testOpenPGPInline()
    {
        MimeTreeParser::ObjectTreeParser otp;
        otp.parseObjectTree(readMailFromFile("openpgp-inline-charset-encrypted.mbox"));
        otp.print();
        otp.decryptParts();
        otp.print();
        auto partList = otp.collectContentParts();
        QCOMPARE(partList.size(), 1);
        auto part = partList[0].dynamicCast<MimeTreeParser::MessagePart>();
        QVERIFY(bool(part));
        QCOMPARE(part->charset(), QStringLiteral("ISO-8859-1").toLocal8Bit());
        QCOMPARE(part->text(), QString::fromUtf8("asdasd asd asd asdf sadf sdaf sadf öäü"));

        QCOMPARE(part->encryptions().size(), 1);
        QCOMPARE(part->signatures().size(), 1);
        QCOMPARE(otp.collectAttachmentParts().size(), 0);
    }

    void testOpenPPGInlineWithNonEncText()
    {
        MimeTreeParser::ObjectTreeParser otp;
        otp.parseObjectTree(readMailFromFile("openpgp-inline-encrypted+nonenc.mbox"));
        otp.print();
        otp.decryptParts();
        otp.print();
        auto partList = otp.collectContentParts();
        QCOMPARE(partList.size(), 1);
        auto part1 = partList[0].dynamicCast<MimeTreeParser::MessagePart>();
        QVERIFY(bool(part1));
        QCOMPARE(part1->text(), QStringLiteral("Not encrypted not signed :(\n\nsome random text"));
        //TODO test if we get the proper subparts with the appropriate encryptions
        QCOMPARE(part1->charset(), QStringLiteral("us-ascii").toLocal8Bit());

        QCOMPARE(part1->encryptionState(), MimeTreeParser::KMMsgPartiallyEncrypted);
        QCOMPARE(part1->signatureState(), MimeTreeParser::KMMsgNotSigned);

        // QCOMPARE(part1->text(), QStringLiteral("Not encrypted not signed :(\n\n"));
        // QCOMPARE(part1->charset(), QStringLiteral("us-ascii").toLocal8Bit());
        // QCOMPARE(contentList[1]->content(), QStringLiteral("some random text").toLocal8Bit());
        // QCOMPARE(contentList[1]->charset(), QStringLiteral("us-ascii").toLocal8Bit());
        // QCOMPARE(contentList[1]->encryptions().size(), 1);
        // QCOMPARE(contentList[1]->signatures().size(), 0);
        QCOMPARE(otp.collectAttachmentParts().size(), 0);
    }

    void testEncryptionBlock()
    {
        MimeTreeParser::ObjectTreeParser otp;
        otp.parseObjectTree(readMailFromFile("openpgp-encrypted-attachment-and-non-encrypted-attachment.mbox"));
        otp.print();
        otp.decryptParts();
        otp.print();
        auto partList = otp.collectContentParts();
        QCOMPARE(partList.size(), 1);
        auto part1 = partList[0].dynamicCast<MimeTreeParser::MessagePart>();
        QVERIFY(bool(part1));
        QCOMPARE(part1->encryptions().size(), 1);
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
        otp.decryptParts();
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
        QCOMPARE(part->encryptions().size(), 0);
        QCOMPARE(part->signatures().size(), 0);
        QCOMPARE(otp.collectAttachmentParts().size(), 1);
    }

    void testAttachmentPart()
    {
        MimeTreeParser::ObjectTreeParser otp;
        otp.parseObjectTree(readMailFromFile("attachment.mbox"));
        otp.print();
        auto partList = otp.collectAttachmentParts();
        QCOMPARE(partList.size(), 1);
        auto part = partList[0].dynamicCast<MimeTreeParser::MessagePart>();
        QVERIFY(bool(part));
        auto att = part->node();
        qWarning() << "Attachment type: " << att->contentType(true)->mimeType();
        QCOMPARE(part->mimeType(), QByteArray("image/jpeg"));
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

    void testOpenPGPInlineError()
    {
        MimeTreeParser::ObjectTreeParser otp;
        otp.parseObjectTree(readMailFromFile("inlinepgpgencrypted-error.mbox"));
        otp.print();
        otp.decryptParts();
        otp.print();
        auto partList = otp.collectContentParts();
        QCOMPARE(partList.size(), 1);
        auto part = partList[0].dynamicCast<MimeTreeParser::EncryptedMessagePart>();
        QVERIFY(bool(part));
        QVERIFY(part->error());
    }

    void testEncapsulated()
    {
        MimeTreeParser::ObjectTreeParser otp;
        otp.parseObjectTree(readMailFromFile("encapsulated-with-attachment.mbox"));
        otp.decryptParts();
        auto partList = otp.collectContentParts();
        QCOMPARE(partList.size(), 2);
        auto part = partList[1].dynamicCast<MimeTreeParser::EncapsulatedRfc822MessagePart>();
        QVERIFY(bool(part));
        QCOMPARE(part->from(), QLatin1String("Thomas McGuire <dontspamme@gmx.net>"));
        QCOMPARE(part->date().toString(), QLatin1String("Wed Aug 5 10:57:58 2009 GMT+0200"));
        auto subPartList = otp.collectContentParts(part);
        QCOMPARE(subPartList.size(), 1);
        qWarning() << subPartList[0]->metaObject()->className();
        auto subPart = subPartList[0].dynamicCast<MimeTreeParser::TextMessagePart>();
        QVERIFY(bool(subPart));
    }

    void test8bitEncodedInPlaintext()
    {
        MimeTreeParser::ObjectTreeParser otp;
        otp.parseObjectTree(readMailFromFile("8bitencoded.mbox"));
        QVERIFY(otp.plainTextContent().contains(QString::fromUtf8("Why Pisa’s Tower")));
        QVERIFY(otp.htmlContent().contains(QString::fromUtf8("Why Pisa’s Tower")));
    }

    void testInlineSigned()
    {
        MimeTreeParser::ObjectTreeParser otp;
        otp.parseObjectTree(readMailFromFile("openpgp-inline-signed.mbox"));
        otp.decryptParts();
        auto partList = otp.collectContentParts();
        QCOMPARE(partList.size(), 1);
        auto part = partList[0].dynamicCast<MimeTreeParser::MessagePart>();
        QCOMPARE(part->signatures().size(), 1);
        QCOMPARE(part->encryptionState(), MimeTreeParser::KMMsgNotEncrypted);
        QCOMPARE(part->signatureState(), MimeTreeParser::KMMsgFullySigned);

        auto signaturePart = part->signatures().first();
        QCOMPARE(signaturePart->partMetaData()->isGoodSignature, true);
        QCOMPARE(signaturePart->partMetaData()->keyIsTrusted, true);
        QCOMPARE(signaturePart->partMetaData()->keyMissing, false);
        QCOMPARE(signaturePart->partMetaData()->keyExpired, false);
        QCOMPARE(signaturePart->partMetaData()->keyRevoked, false);
        QCOMPARE(signaturePart->partMetaData()->sigExpired, false);
        QCOMPARE(signaturePart->partMetaData()->crlMissing, false);
        QCOMPARE(signaturePart->partMetaData()->crlTooOld, false);
        QCOMPARE(signaturePart->partMetaData()->keyId, QByteArray{"8D9860C58F246DE6"});
        QCOMPARE(signaturePart->partMetaData()->signer, {"unittest key (no password) <test@kolab.org>"});
        QCOMPARE(signaturePart->partMetaData()->signerMailAddresses, QStringList{{"test@kolab.org"}});
    }

    void testEncryptedAndSigned()
    {
        MimeTreeParser::ObjectTreeParser otp;
        otp.parseObjectTree(readMailFromFile("openpgp-encrypted+signed.mbox"));
        otp.decryptParts();
        auto partList = otp.collectContentParts();
        QCOMPARE(partList.size(), 1);
        auto part = partList[0].dynamicCast<MimeTreeParser::MessagePart>();
        QCOMPARE(part->signatures().size(), 1);
        QCOMPARE(part->encryptions().size(), 1);
        QCOMPARE(part->encryptionState(), MimeTreeParser::KMMsgFullyEncrypted);
        QCOMPARE(part->signatureState(), MimeTreeParser::KMMsgFullySigned);
        QVERIFY(otp.plainTextContent().contains(QString::fromUtf8("encrypted message text")));

        auto signaturePart = part->signatures().first();
        QCOMPARE(signaturePart->partMetaData()->keyId, QByteArray{"8D9860C58F246DE6"});
        QCOMPARE(signaturePart->partMetaData()->isGoodSignature, true);
    }

    void testOpenpgpMultipartEmbedded()
    {
        MimeTreeParser::ObjectTreeParser otp;
        otp.parseObjectTree(readMailFromFile("openpgp-multipart-embedded.mbox"));
        otp.print();
        otp.decryptParts();
        otp.print();
        auto partList = otp.collectContentParts();
        QCOMPARE(partList.size(), 1);
        auto part = partList[0].dynamicCast<MimeTreeParser::MessagePart>();
        QCOMPARE(part->encryptions().size(), 1);
        QCOMPARE(part->encryptionState(), MimeTreeParser::KMMsgFullyEncrypted);
        QCOMPARE(otp.plainTextContent(), QString::fromUtf8("sdflskjsdf\n\n-- \nThis is a HTML signature.\n"));
    }

    void testOpenpgpMultipartEmbeddedSigned()
    {
        MimeTreeParser::ObjectTreeParser otp;
        otp.parseObjectTree(readMailFromFile("openpgp-multipart-embedded-signed.mbox"));
        otp.decryptParts();
        auto partList = otp.collectContentParts();
        QCOMPARE(partList.size(), 1);
        auto part = partList[0].dynamicCast<MimeTreeParser::MessagePart>();
        QCOMPARE(part->encryptions().size(), 1);
        QCOMPARE(part->signatures().size(), 1);
        QCOMPARE(part->encryptionState(), MimeTreeParser::KMMsgFullyEncrypted);
        QCOMPARE(part->signatureState(), MimeTreeParser::KMMsgFullySigned);
        QCOMPARE(otp.plainTextContent(), QString::fromUtf8("test\n\n-- \nThis is a HTML signature.\n"));

        auto signaturePart = part->signatures().first();
        QVERIFY(signaturePart->partMetaData()->keyId.endsWith(QByteArray{"2E3B7787B1B75920"}));
        //We lack the public key for this message
        QCOMPARE(signaturePart->partMetaData()->isGoodSignature, false);
        QCOMPARE(signaturePart->partMetaData()->keyMissing, true);
    }

    void testAppleHtmlWithAttachments()
    {
        MimeTreeParser::ObjectTreeParser otp;
        otp.parseObjectTree(readMailFromFile("applehtmlwithattachments.mbox"));
        otp.decryptParts();
        auto partList = otp.collectContentParts();
        QCOMPARE(partList.size(), 1);
        auto part = partList[0].dynamicCast<MimeTreeParser::AlternativeMessagePart>();
        QVERIFY(part);
        QCOMPARE(part->encryptions().size(), 0);
        QCOMPARE(part->signatures().size(), 0);
        QVERIFY(part->isHtml());
        QCOMPARE(otp.plainTextContent(), QString::fromUtf8("Hi,\n\nThis is an HTML message with attachments.\n\nCheers,\nChristian"));
        QCOMPARE(otp.htmlContent(), QString::fromUtf8("<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=us-ascii\"></head><body style=\"word-wrap: break-word; -webkit-nbsp-mode: space; line-break: after-white-space;\" class=\"\"><meta http-equiv=\"Content-Type\" content=\"text/html; charset=us-ascii\" class=\"\"><div style=\"word-wrap: break-word; -webkit-nbsp-mode: space; line-break: after-white-space;\" class=\"\">Hi,<div class=\"\"><br class=\"\"></div><blockquote style=\"margin: 0 0 0 40px; border: none; padding: 0px;\" class=\"\"><div class=\"\">This is an <b class=\"\">HTML</b> message with attachments.</div></blockquote><div class=\"\"><br class=\"\"></div><div class=\"\">Cheers,</div><div class=\"\">Christian<img apple-inline=\"yes\" id=\"B9EE68A9-83CA-41CD-A3E4-E5BA301F797A\" class=\"\" src=\"cid:F5B62D1D-E4EC-4C59-AA5A-708525C2AC3C\"></div></div></body></html>"));

        auto attachments = otp.collectAttachmentParts();
        QCOMPARE(attachments.size(), 1);
    }

    void testAppleHtmlWithAttachmentsMixed()
    {
        MimeTreeParser::ObjectTreeParser otp;
        otp.parseObjectTree(readMailFromFile("applehtmlwithattachmentsmixed.mbox"));
        otp.decryptParts();
        otp.print();
        auto partList = otp.collectContentParts();
        QCOMPARE(partList.size(), 1);
        auto part = partList[0].dynamicCast<MimeTreeParser::AlternativeMessagePart>();
        QVERIFY(part);
        QCOMPARE(part->encryptions().size(), 0);
        QCOMPARE(part->signatures().size(), 0);
        QVERIFY(part->isHtml());
        QCOMPARE(otp.plainTextContent(), QString::fromUtf8("Hello\n\n\n\nRegards\n\nFsdfsdf"));
        QCOMPARE(otp.htmlContent(), QString::fromUtf8("<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=us-ascii\"></head><body style=\"word-wrap: break-word; -webkit-nbsp-mode: space; line-break: after-white-space;\" class=\"\"><strike class=\"\">Hello</strike><div class=\"\"><br class=\"\"></div><div class=\"\"></div></body></html><html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=us-ascii\"></head><body style=\"word-wrap: break-word; -webkit-nbsp-mode: space; line-break: after-white-space;\" class=\"\"><div class=\"\"></div><div class=\"\"><br class=\"\"></div><div class=\"\"><b class=\"\">Regards</b></div><div class=\"\"><b class=\"\"><br class=\"\"></b></div><div class=\"\">Fsdfsdf</div></body></html>"));

        auto attachments = otp.collectAttachmentParts();
        QCOMPARE(attachments.size(), 1);
    }

    void testInvitation()
    {
        MimeTreeParser::ObjectTreeParser otp;
        otp.parseObjectTree(readMailFromFile("invitation.mbox"));
        otp.decryptParts();
        otp.print();
        auto partList = otp.collectContentParts();
        QCOMPARE(partList.size(), 1);
        auto part = partList[0].dynamicCast<MimeTreeParser::AlternativeMessagePart>();
        QVERIFY(part);
        QCOMPARE(part->encryptions().size(), 0);
        QCOMPARE(part->signatures().size(), 0);
        QVERIFY(!part->isHtml());
        QVERIFY(part->availableModes().contains(MimeTreeParser::Util::MultipartIcal));

        auto attachments = otp.collectAttachmentParts();
        QCOMPARE(attachments.size(), 0);
    }

    void testGmailInvitation()
    {
        MimeTreeParser::ObjectTreeParser otp;
        otp.parseObjectTree(readMailFromFile("gmail-invitation.mbox"));
        otp.decryptParts();
        otp.print();
        auto partList = otp.collectContentParts();
        QCOMPARE(partList.size(), 1);
        auto part = partList[0].dynamicCast<MimeTreeParser::AlternativeMessagePart>();
        QVERIFY(part);
        QCOMPARE(part->encryptions().size(), 0);
        QCOMPARE(part->signatures().size(), 0);
        QVERIFY(part->isHtml());
        QVERIFY(part->availableModes().contains(MimeTreeParser::Util::MultipartIcal));

        auto attachments = otp.collectAttachmentParts();
        QCOMPARE(attachments.size(), 1);
    }
};

QTEST_GUILESS_MAIN(MimeTreeParserTest)
#include "mimetreeparsertest.moc"
