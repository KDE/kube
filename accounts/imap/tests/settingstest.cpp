#include <QTest>
#include <QDebug>
#include <QSignalSpy>
#include <functional>
#include <QStandardPaths>
#include <QDir>
#include <sink/test.h>
#include <sink/store.h>

#include "imapsettings.h"

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
        auto imapUsername = QString("username");
        auto imapPassword = QString("password");
        auto smtpServer = QString("smtpserver");
        auto smtpUsername = QString("username");
        auto smtpPassword = QString("password");
        auto username = QString("username");
        auto emailAddress = QString("emailAddress");

        ImapSettings settings;
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

        Sink::Store::fetchAll<Sink::ApplicationDomain::SinkResource>(Sink::Query()).then<void, QList<Sink::ApplicationDomain::SinkResource::Ptr>>([](const QList<Sink::ApplicationDomain::SinkResource::Ptr> &resources) {
            QCOMPARE(resources.size(), 2);
        })
        .exec().waitForFinished();

        //Ensure we can read back all the information using the accountid
        {
            ImapSettings readSettings;
            QSignalSpy spy(&readSettings, &ImapSettings::imapResourceChanged);
            QSignalSpy spy1(&readSettings, &ImapSettings::smtpResourceChanged);
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
            QCOMPARE(readSettings.property("userName").toString(), smtpUsername);
            QCOMPARE(readSettings.property("emailAddress").toString(), emailAddress);
        }

        {
            ImapSettings settings;
            QSignalSpy spy(&settings, &ImapSettings::imapResourceChanged);
            settings.setAccountIdentifier(accountId);
            QTRY_COMPARE(spy.count(), 2);
            settings.remove();
        }

        Sink::Store::fetchAll<Sink::ApplicationDomain::SinkResource>(Sink::Query()).then<void, QList<Sink::ApplicationDomain::SinkResource::Ptr>>([](const QList<Sink::ApplicationDomain::SinkResource::Ptr> &resources) {
            QCOMPARE(resources.size(), 0);
        })
        .exec().waitForFinished();
    }
};

QTEST_GUILESS_MAIN(SettingsTest)
#include "settingstest.moc"
