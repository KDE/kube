#include <QTest>
#include <QDebug>
#include <QSignalSpy>
#include <functional>
#include <QStandardPaths>
#include <QDir>
#include <QtWebEngine>

#include <QGpgME/KeyListJob>
#include <QGpgME/Protocol>
#include <gpgme++/key.h>
#include <gpgme++/keylistresult.h>

#include "mailtemplates.h"
#include "mailcrypto.h"

static KMime::Content *getSubpart(KMime::Content *msg, const QByteArray &mimeType)
{
    for (const auto c : msg->contents()) {
        if (c->contentType(false)->mimeType() == mimeType) {
            return c;
        }
    }
    return nullptr;
}

static std::vector< GpgME::Key, std::allocator< GpgME::Key > > getKeys(bool smime = false)
{
    QGpgME::KeyListJob *job = nullptr;

    if (smime) {
        const QGpgME::Protocol *const backend = QGpgME::smime();
        Q_ASSERT(backend);
        job = backend->keyListJob(/* remote = */ false);
    } else {
        const QGpgME::Protocol *const backend = QGpgME::openpgp();
        Q_ASSERT(backend);
        job = backend->keyListJob(/* remote = */ false);
    }
    Q_ASSERT(job);

    std::vector< GpgME::Key > keys;
    GpgME::KeyListResult res = job->exec(QStringList(), /* secretOnly = */ true, keys);

    if (!smime) {
        Q_ASSERT(keys.size() == 3);
    }

    Q_ASSERT(!res.error());

    /*
    qDebug() << "got private keys:" << keys.size();

    for (std::vector< GpgME::Key >::iterator i = keys.begin(); i != keys.end(); ++i) {
        qDebug() << "key isnull:" << i->isNull() << "isexpired:" << i->isExpired();
        qDebug() << "key numuserIds:" << i->numUserIDs();
        for (uint k = 0; k < i->numUserIDs(); ++k) {
            qDebug() << "userIDs:" << i->userID(k).email();
        }
    }
    */

    return keys;
}

static QByteArray readMailFromFile(const QString &mailFile)
{
    Q_ASSERT(!QString::fromLatin1(MAIL_DATA_DIR).isEmpty());
    QFile file(QLatin1String(MAIL_DATA_DIR) + QLatin1Char('/') + mailFile);
    file.open(QIODevice::ReadOnly);
    Q_ASSERT(file.isOpen());
    return file.readAll();
}


static KMime::Message::Ptr readMail(const QString &mailFile)
{
    auto msg = KMime::Message::Ptr::create();
    msg->setContent(readMailFromFile(mailFile));
    msg->parse();
    return msg;
}

static QString removeFirstLine(const QString &s)
{
    return s.mid(s.indexOf("\n") + 1);
}

static QString normalize(const QString &s)
{
    auto text = s;
    text.replace(">", "");
    text.replace("\n", "");
    text.replace("=", "");
    text.replace(" ", "");
    return text;
}

static QString unquote(const QString &s)
{
    auto text = s;
    text.replace("> ", "");
    return text;
}

class MailTemplateTest : public QObject
{
    Q_OBJECT
private slots:

    void initTestCase()
    {
        QtWebEngine::initialize();
    }

    void testPlainReply()
    {
        auto msg = readMail("plaintext.mbox");
        KMime::Message::Ptr result;
        MailTemplates::reply(msg, [&] (const KMime::Message::Ptr &r) {
            result = r;
        });
        QTRY_VERIFY(result);
        QCOMPARE(normalize(removeFirstLine(result->body())), normalize(msg->body()));
        QCOMPARE(result->to()->addresses(), {{"konqi@example.org"}});
        QCOMPARE(result->subject()->asUnicodeString(), {"RE: A random subject with alternative contenttype"});
    }

    void testHtmlReply()
    {
        auto msg = readMail("html.mbox");
        KMime::Message::Ptr result;
        MailTemplates::reply(msg, [&] (const KMime::Message::Ptr &r) {
            result = r;
        });
        QTRY_VERIFY(result);
        QCOMPARE(unquote(removeFirstLine(result->body())), QLatin1String("HTML text"));
    }

    void testHtml8BitEncodedReply()
    {
        auto msg = readMail("8bitencoded.mbox");
        KMime::Message::Ptr result;
        MailTemplates::reply(msg, [&] (const KMime::Message::Ptr &r) {
            result = r;
        });
        QTRY_VERIFY(result);
        QVERIFY(MailTemplates::plaintextContent(result).contains(QString::fromUtf8("Why Pisa’s Tower")));
    }

    void testMultipartSignedReply()
    {
        auto msg = readMail("openpgp-signed-mailinglist.mbox");
        KMime::Message::Ptr result;
        MailTemplates::reply(msg, [&] (const KMime::Message::Ptr &r) {
            result = r;
        });
        QTRY_VERIFY(result);
        auto content = removeFirstLine(result->body());
        QVERIFY(!content.isEmpty());
        QVERIFY(content.contains("i noticed a new branch"));
    }

    void testMultipartAlternativeReply()
    {
        auto msg = readMail("alternative.mbox");
        KMime::Message::Ptr result;
        MailTemplates::reply(msg, [&] (const KMime::Message::Ptr &r) {
            result = r;
        });
        QTRY_VERIFY(result);
        auto content = removeFirstLine(result->body());
        QVERIFY(!content.isEmpty());
        QCOMPARE(unquote(content), QLatin1String("If you can see this text it means that your email client couldn't display our newsletter properly.\nPlease visit this link to view the newsletter on our website: http://www.gog.com/newsletter/\n"));
    }

    void testAttachmentReply()
    {
        auto msg = readMail("plaintextattachment.mbox");
        KMime::Message::Ptr result;
        MailTemplates::reply(msg, [&] (const KMime::Message::Ptr &r) {
            result = r;
        });
        QTRY_VERIFY(result);
        auto content = removeFirstLine(result->body());
        QVERIFY(!content.isEmpty());
        QCOMPARE(unquote(content), QLatin1String("sdlkjsdjf"));
    }

    void testMultiRecipientReply()
    {
        auto msg = readMail("multirecipients.mbox");
        KMime::Message::Ptr result;
        MailTemplates::reply(msg, [&] (const KMime::Message::Ptr &r) {
            result = r;
        });
        QTRY_VERIFY(result);
        auto content = removeFirstLine(result->body());
        QVERIFY(!content.isEmpty());
        QCOMPARE(unquote(content), QLatin1String("test"));
        QCOMPARE(result->to()->addresses(), {{"konqi@example.org"}});
        auto l = QVector<QByteArray>{{"release-team@kde.org"}, {"kde-devel@kde.org"}};
        QCOMPARE(result->cc()->addresses(), l);
    }

    void testMultiRecipientReplyFilteringMe()
    {
        KMime::Types::AddrSpecList me;
        KMime::Types::Mailbox mb;
        mb.setAddress("release-team@kde.org");
        me << mb.addrSpec();

        auto msg = readMail("multirecipients.mbox");
        KMime::Message::Ptr result;
        MailTemplates::reply(msg, [&] (const KMime::Message::Ptr &r) {
            result = r;
        }, me);
        QTRY_VERIFY(result);
        auto content = removeFirstLine(result->body());
        QVERIFY(!content.isEmpty());
        QCOMPARE(unquote(content), QLatin1String("test"));
        QCOMPARE(result->to()->addresses(), {{"konqi@example.org"}});
        auto l = QVector<QByteArray>{{"kde-devel@kde.org"}};
        QCOMPARE(result->cc()->addresses(), l);
    }

    void testForwardAsAttachment()
    {
        auto msg = readMail("plaintext.mbox");
        KMime::Message::Ptr result;
        MailTemplates::forward(msg, [&] (const KMime::Message::Ptr &r) {
            result = r;
        });
        QTRY_VERIFY(result);
        QCOMPARE(result->subject(false)->asUnicodeString(), {"FW: A random subject with alternative contenttype"});
        QCOMPARE(result->to()->addresses(), {});
        QCOMPARE(result->cc()->addresses(), {});

        auto attachments = result->attachments();
        QCOMPARE(attachments.size(), 1);
        auto attachment = attachments[0];
        QCOMPARE(attachment->contentDisposition(false)->disposition(), KMime::Headers::CDinline);
        QCOMPARE(attachment->contentDisposition(false)->filename(), {"A random subject with alternative contenttype.eml"});
        QVERIFY(attachment->bodyIsMessage());

        attachment->parse();
        auto origMsg = attachment->bodyAsMessage();
        QCOMPARE(origMsg->subject(false)->asUnicodeString(), {"A random subject with alternative contenttype"});
    }

    void testEncryptedForwardAsAttachment()
    {
        auto msg = readMail("openpgp-encrypted.mbox");
        KMime::Message::Ptr result;
        MailTemplates::forward(msg, [&](const KMime::Message::Ptr &r) { result = r; });
        QTRY_VERIFY(result);
        QCOMPARE(result->subject(false)->asUnicodeString(), {"FW: OpenPGP encrypted"});
        QCOMPARE(result->to()->addresses(), {});
        QCOMPARE(result->cc()->addresses(), {});

        auto attachments = result->attachments();
        QCOMPARE(attachments.size(), 1);
        auto attachment = attachments[0];
        QCOMPARE(attachment->contentDisposition(false)->disposition(), KMime::Headers::CDinline);
        QCOMPARE(attachment->contentDisposition(false)->filename(), {"OpenPGP encrypted.eml"});
        QVERIFY(attachment->bodyIsMessage());

        attachment->parse();
        auto origMsg = attachment->bodyAsMessage();
        QCOMPARE(origMsg->subject(false)->asUnicodeString(), {"OpenPGP encrypted"});
    }

    void testEncryptedWithAttachmentsForwardAsAttachment()
    {
        auto msg = readMail("openpgp-encrypted-two-attachments.mbox");
        KMime::Message::Ptr result;
        MailTemplates::forward(msg, [&](const KMime::Message::Ptr &r) { result = r; });
        QTRY_VERIFY(result);
        QCOMPARE(result->subject(false)->asUnicodeString(), {"FW: OpenPGP encrypted with 2 text attachments"});
        QCOMPARE(result->to()->addresses(), {});
        QCOMPARE(result->cc()->addresses(), {});

        auto attachments = result->attachments();
        QCOMPARE(attachments.size(), 1);
        auto attachment = attachments[0];
        QCOMPARE(attachment->contentDisposition(false)->disposition(), KMime::Headers::CDinline);
        QCOMPARE(attachment->contentDisposition(false)->filename(), {"OpenPGP encrypted with 2 text attachments.eml"});
        QVERIFY(attachment->bodyIsMessage());

        attachment->parse();
        auto origMsg = attachment->bodyAsMessage();
        QCOMPARE(origMsg->subject(false)->asUnicodeString(), {"OpenPGP encrypted with 2 text attachments"});

        auto attattachments = origMsg->attachments();
        QCOMPARE(attattachments.size(), 2);
        QCOMPARE(attattachments[0]->contentDisposition(false)->filename(), {"attachment1.txt"});
        QCOMPARE(attattachments[1]->contentDisposition(false)->filename(), {"attachment2.txt"});
    }

    void testCreatePlainMail()
    {
        QStringList to = {{"to@example.org"}};
        QStringList cc = {{"cc@example.org"}};
        QStringList bcc = {{"bcc@example.org"}};;
        KMime::Types::Mailbox from;
        from.fromUnicodeString("from@example.org");
        QString subject = "subject";
        QString body = "body";
        QList<Attachment> attachments;

        auto result = MailTemplates::createMessage({}, to, cc, bcc, from, subject, body, false, attachments);

        QVERIFY(result);
        QCOMPARE(result->subject()->asUnicodeString(), subject);
        QCOMPARE(result->body(), body.toUtf8());
        QVERIFY(result->date(false)->dateTime().isValid());
        QVERIFY(result->contentType()->isMimeType("text/plain"));
        QVERIFY(result->messageID(false) && !result->messageID(false)->isEmpty());
    }

    void testCreateHtmlMail()
    {
        QStringList to = {{"to@example.org"}};
        QStringList cc = {{"cc@example.org"}};
        QStringList bcc = {{"bcc@example.org"}};;
        KMime::Types::Mailbox from;
        from.fromUnicodeString("from@example.org");
        QString subject = "subject";
        QString body = "body";
        QList<Attachment> attachments;

        auto result = MailTemplates::createMessage({}, to, cc, bcc, from, subject, body, true, attachments);

        QVERIFY(result);
        QCOMPARE(result->subject()->asUnicodeString(), subject);
        QVERIFY(result->date(false)->dateTime().isValid());
        QVERIFY(result->contentType()->isMimeType("multipart/alternative"));
        const auto contents = result->contents();
        //1 Plain + 1 Html
        QCOMPARE(contents.size(), 2);
    }

    void testCreatePlainMailWithAttachments()
    {
        QStringList to = {{"to@example.org"}};
        QStringList cc = {{"cc@example.org"}};;
        QStringList bcc = {{"bcc@example.org"}};;
        KMime::Types::Mailbox from;
        from.fromUnicodeString("from@example.org");
        QString subject = "subject";
        QString body = "body";
        QList<Attachment> attachments = {{"name", "filename", "mimetype", true, "inlineAttachment"}, {"name", "filename", "mimetype", false, "nonInlineAttachment"}};

        auto result = MailTemplates::createMessage({}, to, cc, bcc, from, subject, body, false, attachments);

        QVERIFY(result);
        QCOMPARE(result->subject()->asUnicodeString(), subject);
        QVERIFY(result->contentType()->isMimeType("multipart/mixed"));
        QVERIFY(result->date(false)->dateTime().isValid());
        const auto contents = result->contents();
        //1 Plain + 2 Attachments
        QCOMPARE(contents.size(), 3);
        auto p = getSubpart(result.data(), "text/plain");
        QVERIFY(p);
    }

    void testCreateHtmlMailWithAttachments()
    {
        QStringList to = {{"to@example.org"}};
        QStringList cc = {{"cc@example.org"}};;
        QStringList bcc = {{"bcc@example.org"}};;
        KMime::Types::Mailbox from;
        from.fromUnicodeString("from@example.org");
        QString subject = "subject";
        QString body = "body";
        QList<Attachment> attachments = {{"name", "filename", "mimetype", true, "inlineAttachment"}, {"name", "filename", "mimetype", false, "nonInlineAttachment"}};

        auto result = MailTemplates::createMessage({}, to, cc, bcc, from, subject, body, true, attachments);

        QVERIFY(result);
        QCOMPARE(result->subject()->asUnicodeString(), subject);
        QVERIFY(result->contentType()->isMimeType("multipart/mixed"));
        QVERIFY(result->date(false)->dateTime().isValid());
        const auto contents = result->contents();
        //1 alternative + 2 Attachments
        QCOMPARE(contents.size(), 3);
        auto p = getSubpart(result.data(), "multipart/alternative");
        QVERIFY(p);
    }

    void testCreatePlainMailSigned()
    {
        QStringList to = {{"to@example.org"}};
        QStringList cc = {{"cc@example.org"}};;
        QStringList bcc = {{"bcc@example.org"}};;
        KMime::Types::Mailbox from;
        from.fromUnicodeString("from@example.org");
        QString subject = "subject";
        QString body = "body";
        QList<Attachment> attachments;

        std::vector<GpgME::Key> keys = getKeys();

        auto result = MailTemplates::createMessage({}, to, cc, bcc, from, subject, body, false, attachments, keys, {}, keys[0]);

        QVERIFY(result);
        // qWarning() << "---------------------------------";
        // qWarning().noquote() << result->encodedContent();
        // qWarning() << "---------------------------------";
        QCOMPARE(result->subject()->asUnicodeString(), subject);
        QVERIFY(result->date(false)->dateTime().isValid());

        QCOMPARE(result->contentType()->mimeType(), QByteArray{"multipart/mixed"});
        auto resultAttachments = result->attachments();
        QCOMPARE(resultAttachments.size(), 1);
        QCOMPARE(resultAttachments[0]->contentDisposition()->filename(), {"0x8F246DE6.asc"});

        auto signedMessage = result->contents()[0];

        QVERIFY(signedMessage->contentType()->isMimeType("multipart/signed"));

        const auto contents = signedMessage->contents();
        QCOMPARE(contents.size(), 2);
        {
            auto c = contents.at(0);
            QVERIFY(c->contentType()->isMimeType("text/plain"));
        }
        {
            auto c = contents.at(1);
            QVERIFY(c->contentType()->isMimeType("application/pgp-signature"));
        }
    }

    void testCreatePlainMailWithAttachmentsSigned()
    {
        QStringList to = {{"to@example.org"}};
        QStringList cc = {{"cc@example.org"}};;
        QStringList bcc = {{"bcc@example.org"}};;
        KMime::Types::Mailbox from;
        from.fromUnicodeString("from@example.org");
        QString subject = "subject";
        QString body = "body";
        QList<Attachment> attachments = {{"name", "filename", "mimetype", true, "inlineAttachment"}, {"name", "filename", "mimetype", false, "nonInlineAttachment"}};

        std::vector<GpgME::Key> keys = getKeys();

        auto result = MailTemplates::createMessage({}, to, cc, bcc, from, subject, body, false, attachments, keys);

        QVERIFY(result);
        QCOMPARE(result->subject()->asUnicodeString(), subject);
        QVERIFY(result->date(false)->dateTime().isValid());

        QCOMPARE(result->contentType()->mimeType(), QByteArray{"multipart/mixed"});
        auto resultAttachments = result->attachments();
        QCOMPARE(resultAttachments.size(), 3);
        // It seems KMime searches for the attachments using depth-first
        // search, so the public key is last
        QCOMPARE(resultAttachments[2]->contentDisposition()->filename(), {"0x8F246DE6.asc"});

        auto signedMessage = result->contents()[0];

        QVERIFY(signedMessage->contentType()->isMimeType("multipart/signed"));

        const auto contents = signedMessage->contents();
        QCOMPARE(contents.size(), 2);
        {
            auto c = contents.at(0);
            QVERIFY(c->contentType()->isMimeType("multipart/mixed"));
            //1 text + 2 attachments
            QCOMPARE(c->contents().size(), 3);
        }
        {
            auto c = contents.at(1);
            QVERIFY(c->contentType()->isMimeType("application/pgp-signature"));
        }
    }
};

QTEST_MAIN(MailTemplateTest)
#include "mailtemplatetest.moc"
