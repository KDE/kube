#include <QTest>
#include <QDebug>
#include <QSignalSpy>
#include <sink/test.h>
#include <sink/store.h>
#include <sink/log.h>
#include <KMime/Message>

#include <actions/action.h>
#include <actions/context.h>

using namespace Sink;

class SinkActionTest : public QObject
{
    Q_OBJECT
private slots:

    void initTestCase()
    {
        Sink::Test::initTest();
        Sink::Log::setDebugOutputLevel(Sink::Log::Trace);
    }

    void testSaveAsDraftFail()
    {
        Kube::Context context;
        auto future = Kube::Action("org.kde.kube.actions.save-as-draft", context).executeWithResult();

        QTRY_VERIFY(future.isDone());
        //because of empty context
        QVERIFY(future.error());
    }

    void testSaveAsDraftNew()
    {
        auto message = KMime::Message::Ptr::create();
        message->subject(true)->fromUnicodeString(QString::fromLatin1("Foobar"), "utf8");
        message->assemble();

        auto &&account = Test::TestAccount::registerAccount();
        auto folder = account.createEntity<ApplicationDomain::Folder>();
        folder->setProperty("specialpurpose", QVariant::fromValue(QByteArrayList() << "drafts"));

        Kube::Context context;
        context.setProperty("message", QVariant::fromValue(message));
        context.setProperty("accountId", QVariant::fromValue(account.identifier));
        auto future = Kube::Action("org.kde.kube.actions.save-as-draft", context).executeWithResult();

        QTRY_VERIFY(future.isDone());
        QVERIFY(!future.error());
        auto mails = account.entities<Sink::ApplicationDomain::Mail>();
        QCOMPARE(mails.size(), 1);
        auto mail = mails.first();
        QCOMPARE(mail->getProperty("folder").toByteArray(), folder->identifier());
    }
};

QTEST_GUILESS_MAIN(SinkActionTest)
#include "sinkactiontest.moc"
