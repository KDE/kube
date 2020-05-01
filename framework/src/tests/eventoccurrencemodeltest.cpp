
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
            //1st indent level
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
            //2rd indent level
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
        {
            //3rd indent level
            auto event6 = ApplicationDomainType::createEntity<Event>(resource.identifier());
            auto calcoreEvent = QSharedPointer<KCalCore::Event>::create();
            calcoreEvent->setUid("event6");
            calcoreEvent->setSummary("summary6");
            calcoreEvent->setDtStart(start.addDays(1));
            calcoreEvent->setDuration(3600);
            calcoreEvent->setAllDay(false);
            event6.setIcal(KCalCore::ICalFormat().toICalString(calcoreEvent).toUtf8());
            event6.setCalendar(calendar1);
            Sink::Store::create(event6).exec().waitForFinished();
        }
        {
            //Start matches end of previous event
            auto event7 = ApplicationDomainType::createEntity<Event>(resource.identifier());
            auto calcoreEvent = QSharedPointer<KCalCore::Event>::create();
            calcoreEvent->setUid("event7");
            calcoreEvent->setSummary("summary7");
            calcoreEvent->setDtStart(start.addDays(1).addSecs(3600));
            calcoreEvent->setDuration(3600);
            calcoreEvent->setAllDay(false);
            event7.setIcal(KCalCore::ICalFormat().toICalString(calcoreEvent).toUtf8());
            event7.setCalendar(calendar1);
            Sink::Store::create(event7).exec().waitForFinished();
        }

        Sink::ResourceControl::flushMessageQueue(resource.identifier()).exec().waitForFinished();

        {
            const int expectedNumberOfOccurreces = 12;
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
                QTRY_COMPARE(multiDayModel.index(0, 0, {}).data(multiDayModel.roleNames().key("events")).value<QVariantList>().size(), 4);
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
                    QCOMPARE(events.size(), 4);
                    QCOMPARE(events[0].toMap()["indentation"].toInt(), 0);
                    QCOMPARE(events[1].toMap()["indentation"].toInt(), 1);
                    QCOMPARE(events[2].toMap()["indentation"].toInt(), 2);
                    QCOMPARE(events[3].toMap()["indentation"].toInt(), 0);
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

    void testRecurrenceException()
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
            auto event = ApplicationDomainType::createEntity<Event>(resource.identifier());
            auto calcoreEvent = QSharedPointer<KCalCore::Event>::create();
            calcoreEvent->setUid("event");
            calcoreEvent->setSummary("summary2");
            calcoreEvent->setDescription("description");
            calcoreEvent->setDtStart(start.addDays(1));
            calcoreEvent->setDuration(3600);
            calcoreEvent->setAllDay(false);
            calcoreEvent->recurrence()->setDaily(1);
            event.setIcal(KCalCore::ICalFormat().toICalString(calcoreEvent).toUtf8());
            event.setCalendar(calendar1);
            Sink::Store::create(event).exec().waitForFinished();
        }

        //Exception
        {
            auto event = ApplicationDomainType::createEntity<Event>(resource.identifier());
            auto calcoreEvent = QSharedPointer<KCalCore::Event>::create();
            calcoreEvent->setUid("event");
            calcoreEvent->setSummary("summary2");
            calcoreEvent->setDescription("description");
            calcoreEvent->setRecurrenceId(start.addDays(2));
            calcoreEvent->setDtStart(start.addDays(2).addSecs(3600));
            calcoreEvent->setDuration(7200);
            calcoreEvent->setAllDay(false);
            event.setIcal(KCalCore::ICalFormat().toICalString(calcoreEvent).toUtf8());
            event.setCalendar(calendar1);
            Sink::Store::create(event).exec().waitForFinished();
        }

        Sink::ResourceControl::flushMessageQueue(resource.identifier()).exec().waitForFinished();

        {
            EventOccurrenceModel model;
            model.setStart(start.date());
            model.setLength(7);
            model.setCalendarFilter({calendar1.identifier()});
            QTRY_COMPARE(model.rowCount({}), 6);

            auto getOccurrence = [&] (int index) {
                return model.index(index, 0, {}).data(EventOccurrenceModel::EventOccurrence).value<EventOccurrenceModel::Occurrence>();
            };
            QCOMPARE(getOccurrence(0).start, start.addDays(1));
            QCOMPARE(getOccurrence(1).start, start.addDays(2).addSecs(3600)); //The exception
            QCOMPARE(getOccurrence(2).start, start.addDays(3));
        }
    }

};

QTEST_MAIN(EventOccurrenceModelTest)
#include "eventoccurrencemodeltest.moc"
