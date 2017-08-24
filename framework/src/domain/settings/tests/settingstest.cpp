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
        loadIdentity();
    }

    Q_INVOKABLE virtual void save() Q_DECL_OVERRIDE
    {
        saveAccount();
        saveImapResource();
        saveMailtransportResource();
        saveIdentity();
    }

    Q_INVOKABLE virtual void remove() Q_DECL_OVERRIDE
    {
        removeResource(mMailtransportIdentifier);
        removeResource(mImapIdentifier);
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
        auto accountId = "accountid";
        auto imapServer = QString("imapserver");
        auto imapUsername = QString("imapName");
        auto imapPassword = QString("imapPw");
        auto smtpServer = QString("smtpserver");
        auto smtpUsername = QString("smtpName");
        auto smtpPassword = QString("smtpPw");
        auto username = QString("username");
        auto emailAddress = QString("emailAddress");

        TestSettings settings;
        settings.setAccountIdentifier(accountId);
        settings.setProperty("imapServer", imapServer);
        settings.setProperty("imapUsername", imapUsername);
        settings.setProperty("imapPassword", imapPassword);
        settings.setProperty("smtpServer", smtpServer);
        settings.setProperty("smtpUsername", smtpUsername);
        settings.setProperty("smtpPassword", smtpPassword);
        settings.setProperty("userName", username);
        settings.setProperty("emailAddress", emailAddress);
        settings.save();

        Sink::Store::fetchAll<Sink::ApplicationDomain::SinkResource>(Sink::Query()).then([](const QList<Sink::ApplicationDomain::SinkResource::Ptr> &resources) {
            QCOMPARE(resources.size(), 2);
        })
        .exec().waitForFinished();

        //Ensure we can read back all the information using the accountid
        {
            TestSettings readSettings;
            QSignalSpy spy(&readSettings, &TestSettings::imapResourceChanged);
            QSignalSpy spy1(&readSettings, &TestSettings::smtpResourceChanged);
            readSettings.setAccountIdentifier(accountId);
            //Once for clear and once for the new setting
            QTRY_COMPARE(spy.count(), 2);
            QTRY_COMPARE(spy1.count(), 2);
            QVERIFY(!readSettings.accountIdentifier().isEmpty());
            QCOMPARE(readSettings.property("imapServer").toString(), imapServer);
            QCOMPARE(readSettings.property("imapUsername").toString(), imapUsername);
            QCOMPARE(readSettings.property("imapPassword").toString(), imapPassword);
            QCOMPARE(readSettings.property("smtpServer").toString(), smtpServer);
            QCOMPARE(readSettings.property("smtpUsername").toString(), smtpUsername);
            QCOMPARE(readSettings.property("smtpPassword").toString(), smtpPassword);
            QCOMPARE(readSettings.property("userName").toString(), username);
            QCOMPARE(readSettings.property("emailAddress").toString(), emailAddress);
        }

        //Modify all settings
        {
            settings.setProperty("imapServer", imapServer + "mod");
            settings.setProperty("imapUsername", imapUsername + "mod");
            settings.setProperty("imapPassword", imapPassword + "mod");
            settings.setProperty("smtpServer", smtpServer + "mod");
            settings.setProperty("smtpUsername", smtpUsername + "mod");
            settings.setProperty("smtpPassword", smtpPassword + "mod");
            settings.setProperty("userName", username + "mod");
            settings.setProperty("emailAddress", emailAddress + "mod");
            settings.save();
        }

        //Read back settings again
        {
            TestSettings readSettings;
            QSignalSpy spy(&readSettings, &TestSettings::imapResourceChanged);
            QSignalSpy spy1(&readSettings, &TestSettings::smtpResourceChanged);
            readSettings.setAccountIdentifier(accountId);
            //Once for clear and once for the new setting
            QTRY_COMPARE(spy.count(), 2);
            QTRY_COMPARE(spy1.count(), 2);
            QVERIFY(!readSettings.accountIdentifier().isEmpty());
            QCOMPARE(readSettings.property("imapServer").toString(), imapServer + "mod");
            QCOMPARE(readSettings.property("imapUsername").toString(), imapUsername + "mod");
            QCOMPARE(readSettings.property("imapPassword").toString(), imapPassword + "mod");
            QCOMPARE(readSettings.property("smtpServer").toString(), smtpServer + "mod");
            QCOMPARE(readSettings.property("smtpUsername").toString(), smtpUsername + "mod");
            QCOMPARE(readSettings.property("smtpPassword").toString(), smtpPassword + "mod");
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
