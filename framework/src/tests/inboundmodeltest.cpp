
#include <QTest>
#include <QDebug>
#include <QSignalSpy>
#include <sink/test.h>
#include <sink/store.h>
#include <sink/resourcecontrol.h>
#include <KCalCore/Event>
#include <KCalCore/ICalFormat>

#include "inboundmodel.h"

class InboundModelTest : public QObject
{
    Q_OBJECT
private slots:

    void initTestCase()
    {
        Sink::Test::initTest();
    }

    void testInboundModel()
    {
        Sink::ApplicationDomain::DummyResource::create("account1");

        using namespace Sink::ApplicationDomain;
        auto account = ApplicationDomainType::createEntity<SinkAccount>();
        Sink::Store::create(account).exec().waitForFinished();

        auto resource = Sink::ApplicationDomain::DummyResource::create(account.identifier());
        Sink::Store::create(resource).exec().waitForFinished();

        auto calendar1 = ApplicationDomainType::createEntity<Calendar>(resource.identifier());
        calendar1.setEnabled(true);
        Sink::Store::create(calendar1).exec().waitForFinished();

        const QDateTime start{{2018, 04, 17}, {6, 0, 0}};
        {
            auto event1 = ApplicationDomainType::createEntity<Event>(resource.identifier());
            auto calcoreEvent = QSharedPointer<KCalCore::Event>::create();
            calcoreEvent->setUid("event1");
            calcoreEvent->setSummary("summary1");
            calcoreEvent->setDescription("description");
            calcoreEvent->setDtStart(start);
            calcoreEvent->setDuration(3600);
            calcoreEvent->setAllDay(false);
            event1.setIcal(KCalCore::ICalFormat().toICalString(calcoreEvent).toUtf8());
            event1.setCalendar(calendar1);
            Sink::Store::create(event1).exec().waitForFinished();
        }
        {
            auto event2 = ApplicationDomainType::createEntity<Event>(resource.identifier());
            auto calcoreEvent = QSharedPointer<KCalCore::Event>::create();
            calcoreEvent->setUid("event2");
            calcoreEvent->setSummary("summary2");
            calcoreEvent->setDescription("description");
            calcoreEvent->setDtStart(start.addDays(1));
            calcoreEvent->setDuration(3600);
            calcoreEvent->setAllDay(false);
            calcoreEvent->recurrence()->setDaily(2);
            event2.setIcal(KCalCore::ICalFormat().toICalString(calcoreEvent).toUtf8());
            event2.setCalendar(calendar1);
            Sink::Store::create(event2).exec().waitForFinished();
        }

        Sink::ResourceControl::flushMessageQueue(resource.identifier()).exec().waitForFinished();

        InboundModel model;

        model.setCurrentDate(start);
        model.configure(
            {}, // QSet<QString> &_senderBlacklist,
            {}, // QSet<QString> &_toBlacklist,
            {}, // QString &_senderNameContainsFilter,
            {}, // QMap<QString, QString> &_perFolderMimeMessageWhitelistFilter,
            {}, // QList<QRegularExpression> &_messageFilter,
            {}, // QList<QString> &_folderSpecialPurposeBlacklist,
            {}  // QList<QString> &_folderNameBlacklist,
        );

        QTest::qWait(500);
        QTRY_COMPARE(model.rowCount({}), 4);

        {

            auto calcoreEvent = QSharedPointer<KCalCore::Event>::create();
            calcoreEvent->setUid("event1");
            calcoreEvent->setSummary("summary1");
            calcoreEvent->setDescription("description");
            calcoreEvent->setDtStart(start);
            calcoreEvent->setDuration(7200);
            calcoreEvent->setAllDay(false);

            const auto event = model.index(3, 0, {}).data(model.roleNames().key("data")).toMap()["domainObject"].value<Sink::ApplicationDomain::Event::Ptr>();
            event->setIcal(KCalCore::ICalFormat().toICalString(calcoreEvent).toUtf8());

            QSignalSpy resetSpy(&model, &QAbstractItemModel::modelReset);
            QSignalSpy rowsInsertedSpy(&model, SIGNAL(rowsInserted(const QModelIndex &, int, int)));
            QSignalSpy rowsRemovedSpy(&model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)));

            Sink::Store::modify(*event).exec().waitForFinished();
            Sink::ResourceControl::flushMessageQueue(resource.identifier()).exec().waitForFinished();

            QTest::qWait(500);
            QCOMPARE(resetSpy.count(), 0);
            //FIXME we seem to be removing and readding all events twice
            QCOMPARE(rowsRemovedSpy.count(), 8);
            QCOMPARE(rowsInsertedSpy.count(), 8);
        }


        {
            QSignalSpy resetSpy(&model, &QAbstractItemModel::modelReset);
            QSignalSpy rowsInsertedSpy(&model, SIGNAL(rowsInserted(const QModelIndex &, int, int)));
            QSignalSpy rowsRemovedSpy(&model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)));
            model.setCurrentDate(start.addSecs(3600));

            QTest::qWait(500);
            QCOMPARE(resetSpy.count(), 0);
            //FIXME This should just remove 1
            QCOMPARE(rowsRemovedSpy.count(), 4);
            QCOMPARE(rowsInsertedSpy.count(), 3);
        }
    }
};

QTEST_MAIN(InboundModelTest)
#include "inboundmodeltest.moc"
