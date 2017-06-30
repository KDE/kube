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

    void testPlain()
    {
        auto msg = readMail("plaintext.mbox");
        KMime::Message::Ptr result;
        MailTemplates::reply(msg, [&] (const KMime::Message::Ptr &r) {
            result = r;
        });
        QTRY_VERIFY(result);
        QCOMPARE(normalize(removeFirstLine(result->body())), normalize(msg->body()));
    }

    void testHtml()
    {
        auto msg = readMail("html.mbox");
        KMime::Message::Ptr result;
        MailTemplates::reply(msg, [&] (const KMime::Message::Ptr &r) {
            result = r;
        });
        QTRY_VERIFY(result);
        QCOMPARE(unquote(removeFirstLine(result->body())), QLatin1String("HTML text"));
    }

    void testMultipartSigned()
    {
        auto msg = readMail("openpgp-signed-mailinglist.mbox");
        KMime::Message::Ptr result;
        MailTemplates::reply(msg, [&] (const KMime::Message::Ptr &r) {
            result = r;
        });
        QTRY_VERIFY(result);
        auto content = normalize(removeFirstLine(result->body()));
        QVERIFY(!content.isEmpty());
        QEXPECT_FAIL("", "Not implemented yet.", Continue);
        QVERIFY(content.contains("i noticed a new branch"));
    }

};

QTEST_MAIN(MailTemplateTest)
#include "mailtemplatetest.moc"
