
#include <QTest>
#include <QDebug>
#include <QStandardItemModel>
#include <sink/test.h>
#include <sink/store.h>
#include <sink/resourcecontrol.h>
#include <KCalCore/Event>
#include <KCalCore/ICalFormat>

#include "eventmodel.h"
#include "multidayeventmodel.h"
#include "perioddayeventmodel.h"

class EventModelTest : public QObject
{
    Q_OBJECT
private slots:

    void initTestCase()
    {
        Sink::Test::initTest();
    }

    void testEventModel()
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

        Sink::ResourceControl::flushMessageQueue(resource.identifier()).exec().waitForFinished();

        {
            EventModel model;
            model.setStart(start.date());
            model.setLength(7);
            model.setCalendarFilter({calendar1.identifier()});
            QTRY_COMPARE(model.rowCount({}), 7);

            //Check the multidayevent model
            {
                MultiDayEventModel multiDayModel;
                multiDayModel.setModel(&model);
                QTRY_COMPARE(multiDayModel.rowCount({}), 1);
                //All except the first from the recurring event
                //FIXME This test fails sometimes
                // QTRY_COMPARE(multiDayModel.index(0, 0, {}).data(multiDayModel.roleNames().key("events")).value<QVariantList>().size(), 6);
            }

            {
                PeriodDayEventModel multiDayModel;
                multiDayModel.setModel(&model);
                QTRY_COMPARE(multiDayModel.rowCount({}), 7);
                {
                    const auto events = multiDayModel.index(0, 0, {}).data(multiDayModel.roleNames().key("events")).value<QVariantList>();
                    QCOMPARE(events.size(), 1);
                }
                {
                    const auto events = multiDayModel.index(1, 0, {}).data(multiDayModel.roleNames().key("events")).value<QVariantList>();
                    QCOMPARE(events.size(), 1);
                }
                {
                    const auto events = multiDayModel.index(2, 0, {}).data(multiDayModel.roleNames().key("events")).value<QVariantList>();
                    QCOMPARE(events.size(), 1);
                }
            }

            model.setCalendarFilter({});
            QTRY_COMPARE(model.rowCount({}), 0);
        }
    }
};

QTEST_MAIN(EventModelTest)
#include "eventmodeltest.moc"
