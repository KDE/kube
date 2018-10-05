#include <QTest>
#include <QDebug>
#include <QSignalSpy>
#include <functional>
#include <QStandardPaths>
#include <QDir>
#include <sink/test.h>
#include <sink/store.h>

#include <accountsettings.h>


class TestSettings : public AccountSettings
{
    Q_OBJECT

public:
    TestSettings(QObject *parent = 0)
        : AccountSettings{parent}
    {}

    Q_INVOKABLE virtual void load() Q_DECL_OVERRIDE
    {
        loadAccount();
        loadImapResource();
        loadMailtransportResource();
        loadMaildirResource();
        loadCardDavResource();
        loadCalDavResource();
        loadIdentity();
    }

    Q_INVOKABLE virtual void save() Q_DECL_OVERRIDE
    {
        saveAccount();
        saveImapResource();
        saveMailtransportResource();
        saveMaildirResource();
        saveCardDavResource();
        saveCalDavResource();
        saveIdentity();
    }

    Q_INVOKABLE virtual void remove() Q_DECL_OVERRIDE
    {
        removeResource(mMailtransportIdentifier);
        removeResource(mImapIdentifier);
        removeResource(mMaildirIdentifier);
        removeResource(mCardDavIdentifier);
        removeResource(mCalDavIdentifier);
        removeIdentity();
        removeAccount();
    }
};

class SettingsTest : public QObject
{
    Q_OBJECT
private slots:

    void initTestCase()
    {
        Sink::Test::initTest();
    }

    void testLoad()
    {
        auto imapServer = QString("imapserver");
        auto imapUsername = QString("imapName");
        auto smtpServer = QString("smtpserver");
        auto smtpUsername = QString("smtpName");
        auto username = QString("username");
        auto emailAddress = QString("emailAddress");
        auto path = QString("path");
        auto accountName = QString("accountName");
        auto carddavServer = QString("carddavServer");
        auto carddavUsername = QString("carddavUsername");
        auto caldavServer = QString("caldavServer");
        auto caldavUsername = QString("caldavUsername");

        TestSettings settings;
        settings.setProperty("accountType", "test");
        settings.setProperty("accountName", accountName);
        settings.setProperty("imapServer", imapServer);
        settings.setProperty("imapUsername", imapUsername);
        settings.setProperty("smtpServer", smtpServer);
        settings.setProperty("smtpUsername", smtpUsername);
        settings.setProperty("carddavServer", carddavServer);
        settings.setProperty("carddavUsername", carddavUsername);
        settings.setProperty("caldavServer", caldavServer);
        settings.setProperty("caldavUsername", caldavUsername);
        settings.setProperty("path", path);
        settings.setProperty("userName", username);
        settings.setProperty("emailAddress", emailAddress);
        settings.save();

        auto accountId = settings.accountIdentifier();

        Sink::Store::fetchAll<Sink::ApplicationDomain::SinkResource>(Sink::Query()).then([](const QList<Sink::ApplicationDomain::SinkResource::Ptr> &resources) {
            QCOMPARE(resources.size(), 5);
        })
        .exec().waitForFinished();

        //Ensure we can read back all the information using the accountid
        {
            TestSettings readSettings;
            QSignalSpy spy(&readSettings, &TestSettings::imapResourceChanged);
            QSignalSpy spy1(&readSettings, &TestSettings::smtpResourceChanged);
            QSignalSpy spy2(&readSettings, &TestSettings::cardDavResourceChanged);
            QSignalSpy spy3(&readSettings, &TestSettings::changed);
            QSignalSpy spy4(&readSettings, &TestSettings::pathChanged);
            QSignalSpy spy5(&readSettings, &TestSettings::calDavResourceChanged);
            readSettings.setAccountIdentifier(accountId);
            //Once for clear and once for the new setting
            QTRY_COMPARE(spy.count(), 2);
            QTRY_COMPARE(spy1.count(), 2);
            QTRY_COMPARE(spy2.count(), 2);
            QTRY_COMPARE(spy3.count(), 2);
            QTRY_COMPARE(spy4.count(), 2);
            QTRY_COMPARE(spy5.count(), 2);
            QVERIFY(!readSettings.accountIdentifier().isEmpty());
            QCOMPARE(readSettings.property("accountName").toString(), accountName);
            QCOMPARE(readSettings.property("imapServer").toString(), imapServer);
            QCOMPARE(readSettings.property("imapUsername").toString(), imapUsername);
            QCOMPARE(readSettings.property("smtpServer").toString(), smtpServer);
            QCOMPARE(readSettings.property("smtpUsername").toString(), smtpUsername);
            QCOMPARE(readSettings.property("carddavServer").toString(), carddavServer);
            QCOMPARE(readSettings.property("carddavUsername").toString(), carddavUsername);
            QCOMPARE(readSettings.property("caldavServer").toString(), caldavServer);
            QCOMPARE(readSettings.property("caldavUsername").toString(), caldavUsername);
            QCOMPARE(readSettings.property("path").toString(), path);
            QCOMPARE(readSettings.property("userName").toString(), username);
            QCOMPARE(readSettings.property("emailAddress").toString(), emailAddress);
        }

        //Modify all settings
        {
            settings.setProperty("accountName", accountName + "mod");
            settings.setProperty("imapServer", imapServer + "mod");
            settings.setProperty("imapUsername", imapUsername + "mod");
            settings.setProperty("smtpServer", smtpServer + "mod");
            settings.setProperty("smtpUsername", smtpUsername + "mod");
            settings.setProperty("carddavServer", carddavServer + "mod");
            settings.setProperty("carddavUsername", carddavUsername + "mod");
            settings.setProperty("caldavServer", caldavServer + "mod");
            settings.setProperty("caldavUsername", caldavUsername + "mod");
            settings.setProperty("path", path + "mod");
            settings.setProperty("userName", username + "mod");
            settings.setProperty("emailAddress", emailAddress + "mod");
            settings.save();
        }

        //Read back settings again
        {
            TestSettings readSettings;
            QSignalSpy spy(&readSettings, &TestSettings::imapResourceChanged);
            QSignalSpy spy1(&readSettings, &TestSettings::smtpResourceChanged);
            QSignalSpy spy2(&readSettings, &TestSettings::cardDavResourceChanged);
            QSignalSpy spy3(&readSettings, &TestSettings::changed);
            QSignalSpy spy4(&readSettings, &TestSettings::pathChanged);
            QSignalSpy spy5(&readSettings, &TestSettings::calDavResourceChanged);
            readSettings.setAccountIdentifier(accountId);
            //Once for clear and once for the new setting
            QTRY_COMPARE(spy.count(), 2);
            QTRY_COMPARE(spy1.count(), 2);
            QTRY_COMPARE(spy2.count(), 2);
            QTRY_COMPARE(spy3.count(), 2);
            QTRY_COMPARE(spy4.count(), 2);
            QTRY_COMPARE(spy5.count(), 2);
            QVERIFY(!readSettings.accountIdentifier().isEmpty());
            QCOMPARE(readSettings.property("accountName").toString(), accountName + "mod");
            QCOMPARE(readSettings.property("imapServer").toString(), imapServer + "mod");
            QCOMPARE(readSettings.property("imapUsername").toString(), imapUsername + "mod");
            QCOMPARE(readSettings.property("smtpServer").toString(), smtpServer + "mod");
            QCOMPARE(readSettings.property("smtpUsername").toString(), smtpUsername + "mod");
            QCOMPARE(readSettings.property("carddavServer").toString(), carddavServer + "mod");
            QCOMPARE(readSettings.property("carddavUsername").toString(), carddavUsername + "mod");
            QCOMPARE(readSettings.property("caldavServer").toString(), caldavServer + "mod");
            QCOMPARE(readSettings.property("caldavUsername").toString(), caldavUsername + "mod");
            QCOMPARE(readSettings.property("path").toString(), path + "mod");
            QCOMPARE(readSettings.property("userName").toString(), username + "mod");
            QCOMPARE(readSettings.property("emailAddress").toString(), emailAddress + "mod");
        }

        {
            TestSettings settings;
            QSignalSpy spy(&settings, &TestSettings::imapResourceChanged);
            settings.setAccountIdentifier(accountId);
            QTRY_COMPARE(spy.count(), 2);
            settings.remove();
        }

        Sink::Store::fetchAll<Sink::ApplicationDomain::SinkResource>(Sink::Query()).then([](const QList<Sink::ApplicationDomain::SinkResource::Ptr> &resources) {
            QCOMPARE(resources.size(), 0);
        })
        .exec().waitForFinished();
    }
};

QTEST_GUILESS_MAIN(SettingsTest)
#include "settingstest.moc"
