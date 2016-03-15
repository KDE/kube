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

        MaildirSettings settings;
        settings.setAccountIdentifier(accountId);
        settings.setPath(maildirPath);
        settings.save();

        Sink::Store::fetchAll<Sink::ApplicationDomain::SinkResource>(Sink::Query()).then<void, QList<Sink::ApplicationDomain::SinkResource>>([](const QList<Sink::ApplicationDomain::SinkResource> &resources) {
            QCOMPARE(resources.size(), 1);
        })
        .exec().waitForFinished();

        //Ensure we can read back all the information using the accountid
        {
            MaildirSettings readSettings;
            QSignalSpy spy(&readSettings, &MaildirSettings::pathChanged);
            readSettings.setAccountIdentifier(accountId);
            QTRY_VERIFY(spy.count());
            QVERIFY(!readSettings.identifier().isEmpty());
            QCOMPARE(readSettings.path().toString(), maildirPath);
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
