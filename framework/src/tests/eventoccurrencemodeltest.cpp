
#include <QTest>
#include <QDebug>
#include <QStandardItemModel>
#include <QSignalSpy>
#include <sink/test.h>
#include <sink/store.h>
#include <sink/resourcecontrol.h>
#include <KCalendarCore/Event>
#include <KCalendarCore/ICalFormat>

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
            auto calcoreEvent = QSharedPointer<KCalendarCore::Event>::create();
            calcoreEvent->setUid("event1");
            calcoreEvent->setSummary("summary1");
            calcoreEvent->setDescription("description");
            calcoreEvent->setDtStart(start);
            calcoreEvent->setDuration(3600);
            calcoreEvent->setAllDay(false);
            event1.setIcal(KCalendarCore::ICalFormat().toICalString(calcoreEvent).toUtf8());
            event1.setCalendar(calendar1);
            Sink::Store::create(event1).exec().waitForFinished();
        }
        {
            //1st indent level
            auto event2 = ApplicationDomainType::createEntity<Event>(resource.identifier());
            auto calcoreEvent = QSharedPointer<KCalendarCore::Event>::create();
            calcoreEvent->setUid("event2");
            calcoreEvent->setSummary("summary2");
            calcoreEvent->setDescription("description");
            calcoreEvent->setDtStart(start.addDays(1));
            calcoreEvent->setDuration(3600);
            calcoreEvent->setAllDay(false);
            calcoreEvent->recurrence()->setDaily(1);
            event2.setIcal(KCalendarCore::ICalFormat().toICalString(calcoreEvent).toUtf8());
            event2.setCalendar(calendar1);
            Sink::Store::create(event2).exec().waitForFinished();
        }
        {
            //2rd indent level
            auto event3 = ApplicationDomainType::createEntity<Event>(resource.identifier());
            auto calcoreEvent = QSharedPointer<KCalendarCore::Event>::create();
            calcoreEvent->setUid("event3");
            calcoreEvent->setSummary("summary3");
            calcoreEvent->setDtStart(start.addDays(1));
            calcoreEvent->setDuration(3600);
            calcoreEvent->setAllDay(false);
            event3.setIcal(KCalendarCore::ICalFormat().toICalString(calcoreEvent).toUtf8());
            event3.setCalendar(calendar1);
            Sink::Store::create(event3).exec().waitForFinished();
        }
        {
            auto event4 = ApplicationDomainType::createEntity<Event>(resource.identifier());
            auto calcoreEvent = QSharedPointer<KCalendarCore::Event>::create();
            calcoreEvent->setUid("event4");
            calcoreEvent->setSummary("summary4");
            calcoreEvent->setDtStart(start.addDays(2));
            calcoreEvent->setAllDay(true);
            event4.setIcal(KCalendarCore::ICalFormat().toICalString(calcoreEvent).toUtf8());
            event4.setCalendar(calendar1);
            Sink::Store::create(event4).exec().waitForFinished();
        }
        {
            //all day event 2 with duration of two days
            auto event4 = ApplicationDomainType::createEntity<Event>(resource.identifier());
            auto calcoreEvent = QSharedPointer<KCalendarCore::Event>::create();
            calcoreEvent->setUid("event4.1");
            calcoreEvent->setSummary("summary4.1");
            calcoreEvent->setDtStart(start.addDays(2));
            calcoreEvent->setDtEnd(start.addDays(3));
            calcoreEvent->setAllDay(true);
            event4.setIcal(KCalendarCore::ICalFormat().toICalString(calcoreEvent).toUtf8());
            event4.setCalendar(calendar1);
            Sink::Store::create(event4).exec().waitForFinished();
        }
        {
            auto event1 = ApplicationDomainType::createEntity<Event>(resource.identifier());
            auto calcoreEvent = QSharedPointer<KCalendarCore::Event>::create();
            calcoreEvent->setUid("event5");
            calcoreEvent->setSummary("summary5");
            calcoreEvent->setDescription("description");
            calcoreEvent->setDtStart(start);
            calcoreEvent->setDuration(3600);
            calcoreEvent->setAllDay(false);
            event1.setIcal(KCalendarCore::ICalFormat().toICalString(calcoreEvent).toUtf8());
            event1.setCalendar(calendar1);
            Sink::Store::create(event1).exec().waitForFinished();
        }
        {
            //3rd indent level
            auto event6 = ApplicationDomainType::createEntity<Event>(resource.identifier());
            auto calcoreEvent = QSharedPointer<KCalendarCore::Event>::create();
            calcoreEvent->setUid("event6");
            calcoreEvent->setSummary("summary6");
            calcoreEvent->setDtStart(start.addDays(1));
            calcoreEvent->setDuration(3600);
            calcoreEvent->setAllDay(false);
            event6.setIcal(KCalendarCore::ICalFormat().toICalString(calcoreEvent).toUtf8());
            event6.setCalendar(calendar1);
            Sink::Store::create(event6).exec().waitForFinished();
        }
        {
            //Start matches end of previous event
            auto event7 = ApplicationDomainType::createEntity<Event>(resource.identifier());
            auto calcoreEvent = QSharedPointer<KCalendarCore::Event>::create();
            calcoreEvent->setUid("event7");
            calcoreEvent->setSummary("summary7");
            calcoreEvent->setDtStart(start.addDays(1).addSecs(3600));
            calcoreEvent->setDuration(3600);
            calcoreEvent->setAllDay(false);
            event7.setIcal(KCalendarCore::ICalFormat().toICalString(calcoreEvent).toUtf8());
            event7.setCalendar(calendar1);
            Sink::Store::create(event7).exec().waitForFinished();
        }

        Sink::ResourceControl::flushMessageQueue(resource.identifier()).exec().waitForFinished();

        {
            const int expectedNumberOfOccurreces = 13;
            const int numberOfDays = 7;
            EventOccurrenceModel model;

            QSignalSpy initialItemsLoadedSpy(&model, &EventOccurrenceModel::initialItemsLoaded);

            model.setStart(start.date());
            model.setLength(numberOfDays);
            model.setCalendarFilter({calendar1.identifier()});
            QTRY_COMPARE(model.rowCount({}), expectedNumberOfOccurreces);
            QCOMPARE(initialItemsLoadedSpy.count(), 1);

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
                const auto lines = multiDayModel.index(0, 0, {}).data(multiDayModel.roleNames().key("events")).value<QVariantList>();
                QTRY_COMPARE(countEvents(lines), expectedNumberOfOccurreces);
                //We have 6 lines in the first week
                QCOMPARE(lines.size(), 6);
                QCOMPARE(lines[0].toList().size(), 1); //All day event
                QCOMPARE(lines[0].toList()[0].toMap().value("duration").toInt(), 1);
                QCOMPARE(lines[1].toList().size(), 1); //All day event
                QCOMPARE(lines[1].toList()[0].toMap().value("duration").toInt(), 2);
                QCOMPARE(lines[2].toList().size(), 7); //Recurring event summary2/summary3
                QCOMPARE(lines[3].toList().size(), 2); //summary5/summary6
                QCOMPARE(lines[4].toList().size(), 1); //summary2 FIXME why is it on a second row? because above summary3 moves in-between. Try to prefer existing events?
                QCOMPARE(lines[4].toList()[0].toMap().value("text").toString(), "summary2");
                QCOMPARE(lines[5].toList().size(), 1); //summary7
                QCOMPARE(lines[5].toList()[0].toMap().value("text").toString(), "summary7");
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

            //Test modification
            {
                const auto event = model.index(0, 0, {}).data(EventOccurrenceModel::Event).value<Sink::ApplicationDomain::Event::Ptr>();
                auto calcoreEvent = KCalendarCore::ICalFormat().readIncidence(event->getIcal()).dynamicCast<KCalendarCore::Event>();
                calcoreEvent->setSummary("modified");

                event->setIcal(KCalendarCore::ICalFormat().toICalString(calcoreEvent).toUtf8());

                QSignalSpy resetSpy(&model, &QAbstractItemModel::modelReset);
                QSignalSpy rowsInsertedSpy(&model, SIGNAL(rowsInserted(const QModelIndex &, int, int)));
                QSignalSpy rowsRemovedSpy(&model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)));
                QSignalSpy dataChangedSpy(&model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &)));
                QSignalSpy initialItemsLoadedSpy(&model, &EventOccurrenceModel::initialItemsLoaded);

                VERIFYEXEC(Sink::Store::modify(*event));
                VERIFYEXEC(Sink::ResourceControl::flushMessageQueue(resource.identifier()));

                QTest::qWait(200);
                QCOMPARE(resetSpy.count(), 0);
                QCOMPARE(rowsRemovedSpy.count(), 0);
                QCOMPARE(rowsInsertedSpy.count(), 0);
                QCOMPARE(dataChangedSpy.count(), 1);
                QCOMPARE(initialItemsLoadedSpy.count(), 0);
            }

            //Test an empty filter
            model.setCalendarFilter({});
            QTRY_COMPARE(model.rowCount({}), 0);
        }
    }

    void testMultiWeekEvents()
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
            //all day event 2 with duration of two days
            auto event4 = ApplicationDomainType::createEntity<Event>(resource.identifier());
            auto calcoreEvent = QSharedPointer<KCalendarCore::Event>::create();
            calcoreEvent->setUid("event1");
            calcoreEvent->setSummary("summary1");
            calcoreEvent->setDtStart(start.addDays(2));
            calcoreEvent->setDtEnd(start.addDays(9));
            calcoreEvent->setAllDay(true);
            event4.setIcal(KCalendarCore::ICalFormat().toICalString(calcoreEvent).toUtf8());
            event4.setCalendar(calendar1);
            Sink::Store::create(event4).exec().waitForFinished();
        }

        Sink::ResourceControl::flushMessageQueue(resource.identifier()).exec().waitForFinished();

        {
            const int numberOfDays = 7 * 6;
            EventOccurrenceModel model;

            QSignalSpy initialItemsLoadedSpy(&model, &EventOccurrenceModel::initialItemsLoaded);

            model.setStart(start.date());
            model.setLength(numberOfDays);
            model.setCalendarFilter({calendar1.identifier()});
            //FIXME Reverse these tests?
            QTRY_COMPARE(model.rowCount({}), 1);
            QCOMPARE(initialItemsLoadedSpy.count(), 1);

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
                QTRY_COMPARE(multiDayModel.rowCount({}), 6);
                {
                    const auto lines = multiDayModel.index(0, 0, {}).data(multiDayModel.roleNames().key("events")).value<QVariantList>();
                    QCOMPARE(countEvents(lines), 1);
                    QCOMPARE(lines.size(), 1);
                    QCOMPARE(lines[0].toList().size(), 1); //All day event
                    QCOMPARE(lines[0].toList()[0].toMap().value("duration").toInt(), 5);
                }
                {
                    const auto lines = multiDayModel.index(1, 0, {}).data(multiDayModel.roleNames().key("events")).value<QVariantList>();
                    QCOMPARE(countEvents(lines), 1);
                    QCOMPARE(lines.size(), 1);
                    QCOMPARE(lines[0].toList().size(), 1); //All day event
                    QCOMPARE(lines[0].toList()[0].toMap().value("duration").toInt(), 3);
                }
                {
                    const auto lines = multiDayModel.index(2, 0, {}).data(multiDayModel.roleNames().key("events")).value<QVariantList>();
                    QCOMPARE(countEvents(lines), 0);
                    QCOMPARE(lines.size(), 0);
                }
            }
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
            auto calcoreEvent = QSharedPointer<KCalendarCore::Event>::create();
            calcoreEvent->setUid("event");
            calcoreEvent->setSummary("summary2");
            calcoreEvent->setDescription("description");
            calcoreEvent->setDtStart(start.addDays(1));
            calcoreEvent->setDuration(3600);
            calcoreEvent->setAllDay(false);
            calcoreEvent->recurrence()->setDaily(1);
            event.setIcal(KCalendarCore::ICalFormat().toICalString(calcoreEvent).toUtf8());
            event.setCalendar(calendar1);
            Sink::Store::create(event).exec().waitForFinished();
        }

        //Exception
        {
            auto event = ApplicationDomainType::createEntity<Event>(resource.identifier());
            auto calcoreEvent = QSharedPointer<KCalendarCore::Event>::create();
            calcoreEvent->setUid("event");
            calcoreEvent->setSummary("summary2");
            calcoreEvent->setDescription("description");
            calcoreEvent->setRecurrenceId(start.addDays(2));
            calcoreEvent->setDtStart(start.addDays(2).addSecs(3600));
            calcoreEvent->setDuration(7200);
            calcoreEvent->setAllDay(false);
            event.setIcal(KCalendarCore::ICalFormat().toICalString(calcoreEvent).toUtf8());
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


    //Original event on saturday, exception on monday next week
    void testRecurrenceException2()
    {
        Sink::ApplicationDomain::DummyResource::create("account1");

        using namespace Sink::ApplicationDomain;
        auto account = ApplicationDomainType::createEntity<SinkAccount>();
        Sink::Store::create(account).exec().waitForFinished();

        auto resource = Sink::ApplicationDomain::DummyResource::create(account.identifier());
        Sink::Store::create(resource).exec().waitForFinished();

        auto calendar1 = ApplicationDomainType::createEntity<Calendar>(resource.identifier());
        Sink::Store::create(calendar1).exec().waitForFinished();

        //Monthly recurrence
        {
            auto event = ApplicationDomainType::createEntity<Event>(resource.identifier());
            auto calcoreEvent = QSharedPointer<KCalendarCore::Event>::create();
            calcoreEvent->setUid("event");
            calcoreEvent->setSummary("summary2");
            calcoreEvent->setDescription("description");
            calcoreEvent->setDtStart(QDateTime{{2021, 3, 1}, {15, 0, 0}});
            calcoreEvent->setDuration(3600);
            calcoreEvent->setAllDay(false);
            calcoreEvent->recurrence()->setMonthly(1);
            event.setIcal(KCalendarCore::ICalFormat().toICalString(calcoreEvent).toUtf8());
            event.setCalendar(calendar1);
            Sink::Store::create(event).exec().waitForFinished();
        }

        //Recurs on saturday, exception on the following monday
        {
            auto event = ApplicationDomainType::createEntity<Event>(resource.identifier());
            auto calcoreEvent = QSharedPointer<KCalendarCore::Event>::create();
            calcoreEvent->setUid("event");
            calcoreEvent->setSummary("summary2");
            calcoreEvent->setDescription("description");
            calcoreEvent->setRecurrenceId(QDateTime{{2021, 5, 1}, {15, 0, 0}});
            calcoreEvent->setDtStart(QDateTime{{2021, 5, 3}, {15, 0, 0}});
            calcoreEvent->setDuration(7200);
            calcoreEvent->setAllDay(false);
            event.setIcal(KCalendarCore::ICalFormat().toICalString(calcoreEvent).toUtf8());
            event.setCalendar(calendar1);
            Sink::Store::create(event).exec().waitForFinished();
        }

        Sink::ResourceControl::flushMessageQueue(resource.identifier()).exec().waitForFinished();

        //Old week (start on monday before)
        {
            EventOccurrenceModel model;

            model.setStart({2021, 4, 26});
            model.setLength(7);
            model.setCalendarFilter({calendar1.identifier()});

            //We don't have any signal when loading is done
            QTest::qWait(200);

            QCOMPARE(model.rowCount({}), 0);
        }
        //New week (start on monday after the occurrence)
        {
            EventOccurrenceModel model;
            model.setStart({2021, 5, 3});
            model.setLength(7);
            model.setCalendarFilter({calendar1.identifier()});
            QTRY_COMPARE(model.rowCount({}), 1);

            auto getOccurrence = [&] (int index) {
                return model.index(index, 0, {}).data(EventOccurrenceModel::EventOccurrence).value<EventOccurrenceModel::Occurrence>();
            };
            QDateTime expected{{2021, 5, 3}, {15, 0, 0}};
            QCOMPARE(getOccurrence(0).start, expected);
        }
    }

};

QTEST_MAIN(EventOccurrenceModelTest)
#include "eventoccurrencemodeltest.moc"
