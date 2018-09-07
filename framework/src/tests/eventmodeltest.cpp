
#include <QTest>
#include <QDebug>
#include <QStandardItemModel>
#include <sink/test.h>
#include <sink/store.h>
#include <sink/resourcecontrol.h>
#include <KCalCore/Event>
#include <KCalCore/ICalFormat>

#include "eventmodel.h"

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


        auto event1 = ApplicationDomainType::createEntity<Event>(resource.identifier());

        auto calcoreEvent = QSharedPointer<KCalCore::Event>::create();
        calcoreEvent->setUid("event1");
        calcoreEvent->setSummary("summary");
        calcoreEvent->setDescription("description");
        const QDateTime start{{2018, 04, 17}, {6, 0, 0}};
        calcoreEvent->setDtStart(start);
        calcoreEvent->setDuration(3600);
        calcoreEvent->setAllDay(false);
        event1.setIcal(KCalCore::ICalFormat().toICalString(calcoreEvent).toUtf8());
        event1.setCalendar(calendar1);
        Sink::Store::create(event1).exec().waitForFinished();

        Sink::ResourceControl::flushMessageQueue(resource.identifier()).exec().waitForFinished();

        {
            EventModel model;
            model.setStart(start.date());
            model.setLength(7);
            model.setCalendarFilter({calendar1.identifier()});
            QTRY_COMPARE(model.rowCount({}), 1);

            {
                auto idx = model.index(0, 0, {});
                // auto mail = idx.data(MailListModel::DomainObject).value<Mail::Ptr>();
                // QVERIFY(mail);
                // QVERIFY(!mail->getSubject().isEmpty());
            }
        }
    }
};

QTEST_MAIN(EventModelTest)
#include "eventmodeltest.moc"
