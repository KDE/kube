#include <QTest>
#include <QDebug>
#include <QSignalSpy>
#include <functional>
#include <QStandardPaths>
#include <QDir>
#include <QtWebEngine>

#include "mailtemplates.h"

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

    void testCreatePlainMail()
    {
        QStringList to = {{"to@example.org"}};
        QStringList cc = {{"cc@example.org"}};;
        QStringList bcc = {{"bcc@example.org"}};;
        KMime::Types::Mailbox from;
        from.fromUnicodeString("from@example.org");
        QString subject = "subject";
        QString body = "body";
        QList<Attachment> attachments;

        auto result = MailTemplates::createMessage({}, to, cc, bcc, from, subject, body, attachments);

        QVERIFY(result);
        auto content = removeFirstLine(result->body());
        QCOMPARE(result->subject()->asUnicodeString(), subject);
        QCOMPARE(result->body(), body.toUtf8());
        QVERIFY(result->date(false)->dateTime().isValid());
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

        auto result = MailTemplates::createMessage({}, to, cc, bcc, from, subject, body, attachments);

        QVERIFY(result);
        auto content = removeFirstLine(result->body());
        QCOMPARE(result->subject()->asUnicodeString(), subject);
        QVERIFY(result->contentType()->isMimeType("multipart/mixed"));
        QVERIFY(result->date(false)->dateTime().isValid());
        const auto contents = result->contents();
        //1 Plain + 2 Attachments
        QCOMPARE(contents.size(), 3);
    }
};

QTEST_MAIN(MailTemplateTest)
#include "mailtemplatetest.moc"
