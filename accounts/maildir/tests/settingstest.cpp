#include <QtTest>
#include <QDebug>
#include <functional>

#include "maildirsettings.h"

class SettingsTest : public QObject
{
    Q_OBJECT
private slots:

    void initTestCase()
    {
        // Sink::FacadeFactory::instance().resetFactory();
        // ResourceConfig::clear();
        // Sink::Log::setDebugOutputLevel(Sink::Log::Trace);
    }

    void testLoad()
    {
        auto accountId = "accountid";
        auto maildirPath = QDir::tempPath();

        MaildirSettings settings;
        settings.setAccountIdentifier(accountId);
        settings.setPath(maildirPath);
        settings.save();

        //TODO ensure the maildir resource has been created
        //TODO ensure the path has been setup correctly
        //Ensure we can read the configuration correctly
        //Ensure we can remove the account again
    }
};

QTEST_GUILESS_MAIN(SettingsTest)
#include "settingstest.moc"
