#include <QTest>
#include <QDebug>
#include <QSignalSpy>
#include <functional>
#include <QStandardPaths>
#include <QDir>
#include <sink/test.h>
#include <sink/store.h>

#include "maildirsettings.h"

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
        auto maildirPath = QDir::tempPath();
        auto smtpServer = QString("smtpserver");
        auto smtpUsername = QString("username");
        auto smtpPassword = QString("password");

        MaildirSettings settings;
        settings.setAccountIdentifier(accountId);
        settings.setPath(maildirPath);
        settings.setProperty("smtpServer", smtpServer);
        settings.setProperty("smtpUsername", smtpUsername);
        settings.setProperty("smtpPassword", smtpPassword);
        settings.save();

        Sink::Store::fetchAll<Sink::ApplicationDomain::SinkResource>(Sink::Query()).then<void, QList<Sink::ApplicationDomain::SinkResource>>([](const QList<Sink::ApplicationDomain::SinkResource> &resources) {
            QCOMPARE(resources.size(), 2);
        })
        .exec().waitForFinished();

        //Ensure we can read back all the information using the accountid
        {
            MaildirSettings readSettings;
            QSignalSpy spy(&readSettings, &MaildirSettings::pathChanged);
            QSignalSpy spy1(&readSettings, &MaildirSettings::smtpResourceChanged);
            readSettings.setAccountIdentifier(accountId);
            QTRY_VERIFY(spy.count());
            QTRY_VERIFY(spy1.count());
            QVERIFY(!readSettings.accountIdentifier().isEmpty());
            QCOMPARE(readSettings.path().toString(), maildirPath);
            QCOMPARE(readSettings.property("smtpServer").toString(), smtpServer);
            QCOMPARE(readSettings.property("smtpUsername").toString(), smtpUsername);
            QCOMPARE(readSettings.property("smtpPassword").toString(), smtpPassword);
        }

        {
            MaildirSettings settings;
            QSignalSpy spy(&settings, &MaildirSettings::pathChanged);
            settings.setAccountIdentifier(accountId);
            QTRY_VERIFY(spy.count());
            settings.remove();
        }

        Sink::Store::fetchAll<Sink::ApplicationDomain::SinkResource>(Sink::Query()).then<void, QList<Sink::ApplicationDomain::SinkResource>>([](const QList<Sink::ApplicationDomain::SinkResource> &resources) {
            QCOMPARE(resources.size(), 0);
        })
        .exec().waitForFinished();
    }
};

QTEST_GUILESS_MAIN(SettingsTest)
#include "settingstest.moc"
