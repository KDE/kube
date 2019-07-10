
#include <QTest>
#include <QDebug>
#include <QStandardItemModel>
#include <sink/test.h>
#include <sink/store.h>
#include <sink/resourcecontrol.h>
#include <KCalCore/Event>
#include <KCalCore/ICalFormat>

#include "eventoccurrencemodel.h"
#include "multidayeventmodel.h"
#include "perioddayeventmodel.h"

class EventOccurrenceModelTest : public QObject
{
    Q_OBJECT
private slots:

    void initTestCase()
    {
        Sink::Test::initTest();
    }

    void testEventOccurrenceModel()
    {
        Sink::ApplicationDomain::DummyResource::create("account1");

        using namespace Sink::ApplicationDomain;
        auto account = ApplicationDomainType::createEntity<SinkAccount>();
        Sink::Store::create(account).exec().waitForFinished();

        auto resource = Sink::ApplicationDomain::DummyResource::create(account.identifier());
        Sink::Store::create(resource).exec().waitForFinished();

        auto calendar1 = ApplicationDomainType::createEntity<Calendar>(resource.identifier());
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
            calcoreEvent->recurrence()->setDaily(1);
            event2.setIcal(KCalCore::ICalFormat().toICalString(calcoreEvent).toUtf8());
            event2.setCalendar(calendar1);
            Sink::Store::create(event2).exec().waitForFinished();
        }
        {
            auto event3 = ApplicationDomainType::createEntity<Event>(resource.identifier());
            auto calcoreEvent = QSharedPointer<KCalCore::Event>::create();
            calcoreEvent->setUid("event3");
            calcoreEvent->setSummary("summary3");
            calcoreEvent->setDtStart(start.addDays(1));
            calcoreEvent->setDuration(3600);
            calcoreEvent->setAllDay(false);
            event3.setIcal(KCalCore::ICalFormat().toICalString(calcoreEvent).toUtf8());
            event3.setCalendar(calendar1);
            Sink::Store::create(event3).exec().waitForFinished();
        }
        {
            auto event4 = ApplicationDomainType::createEntity<Event>(resource.identifier());
            auto calcoreEvent = QSharedPointer<KCalCore::Event>::create();
            calcoreEvent->setUid("event4");
            calcoreEvent->setSummary("summary4");
            calcoreEvent->setDtStart(start.addDays(2));
            calcoreEvent->setAllDay(true);
            event4.setIcal(KCalCore::ICalFormat().toICalString(calcoreEvent).toUtf8());
            event4.setCalendar(calendar1);
            Sink::Store::create(event4).exec().waitForFinished();
        }
        {
            auto event1 = ApplicationDomainType::createEntity<Event>(resource.identifier());
            auto calcoreEvent = QSharedPointer<KCalCore::Event>::create();
            calcoreEvent->setUid("event5");
            calcoreEvent->setSummary("summary5");
            calcoreEvent->setDescription("description");
            calcoreEvent->setDtStart(start);
            calcoreEvent->setDuration(3600);
            calcoreEvent->setAllDay(false);
            event1.setIcal(KCalCore::ICalFormat().toICalString(calcoreEvent).toUtf8());
            event1.setCalendar(calendar1);
            Sink::Store::create(event1).exec().waitForFinished();
        }

        Sink::ResourceControl::flushMessageQueue(resource.identifier()).exec().waitForFinished();

        {
            const int expectedNumberOfOccurreces = 10;
            const int numberOfDays = 7;
            EventOccurrenceModel model;
            model.setStart(start.date());
            model.setLength(numberOfDays);
            model.setCalendarFilter({calendar1.identifier()});
            QTRY_COMPARE(model.rowCount({}), expectedNumberOfOccurreces);

            auto countEvents = [&] (const QVariantList &lines) {
                int count = 0;
                for (const auto &line : lines) {
                    count += line.toList().size();
                }
                return count;
            };

            //Check the multidayevent model
            {
                MultiDayEventModel multiDayModel;
                multiDayModel.setModel(&model);
                QTRY_COMPARE(multiDayModel.rowCount({}), 1);
                QTRY_COMPARE(countEvents(multiDayModel.index(0, 0, {}).data(multiDayModel.roleNames().key("events")).value<QVariantList>()), expectedNumberOfOccurreces);
                //Count lines
                QTRY_COMPARE(multiDayModel.index(0, 0, {}).data(multiDayModel.roleNames().key("events")).value<QVariantList>().size(), 3);
            }

            {
                PeriodDayEventModel multiDayModel;
                multiDayModel.setModel(&model);
                QTRY_COMPARE(multiDayModel.rowCount({}), numberOfDays);
                {
                    const auto events = multiDayModel.index(0, 0, {}).data(multiDayModel.roleNames().key("events")).value<QVariantList>();
                    QCOMPARE(events.size(), 2);
                    QCOMPARE(events[0].toMap()["indentation"].toInt(), 0);
                    QCOMPARE(events[1].toMap()["indentation"].toInt(), 1);
                }
                {
                    const auto events = multiDayModel.index(1, 0, {}).data(multiDayModel.roleNames().key("events")).value<QVariantList>();
                    QCOMPARE(events.size(), 2);
                    QCOMPARE(events[0].toMap()["indentation"].toInt(), 0);
                    QCOMPARE(events[1].toMap()["indentation"].toInt(), 1);
                }
                {
                    const auto events = multiDayModel.index(2, 0, {}).data(multiDayModel.roleNames().key("events")).value<QVariantList>();
                    QCOMPARE(events.size(), 1);
                    QCOMPARE(events[0].toMap()["indentation"].toInt(), 0);
                }
            }

            model.setCalendarFilter({});
            QTRY_COMPARE(model.rowCount({}), 0);
        }
    }
};

QTEST_MAIN(EventOccurrenceModelTest)
#include "eventoccurrencemodeltest.moc"
