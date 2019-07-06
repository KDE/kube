#include <QTest>
#include <QDebug>
#include <sink/test.h>
#include <sink/store.h>
#include <sink/resourcecontrol.h>

#include <QDateTime>
#include <KCalCore/ICalFormat>
#include <KCalCore/ScheduleMessage>
#include <KCalCore/Event>
#include <KCalCore/Attendee>
#include "invitationcontroller.h"

using namespace Sink::ApplicationDomain;


class InvitationControllerTest : public QObject
{
    Q_OBJECT

    QByteArray resourceId;

    QString createInvitation(const QByteArray &uid)
    {
        auto calcoreEvent = QSharedPointer<KCalCore::Event>::create();
        calcoreEvent->setUid(uid);
        calcoreEvent->setSummary("summary");
        calcoreEvent->setDescription("description");
        calcoreEvent->setLocation("location");
        calcoreEvent->setDtStart(QDateTime::currentDateTime());
        calcoreEvent->setOrganizer("organizer@test.com");
        calcoreEvent->addAttendee(KCalCore::Attendee::Ptr::create("John Doe", "attendee1@test.com", true, KCalCore::Attendee::NeedsAction));

        return KCalCore::ICalFormat{}.createScheduleMessage(calcoreEvent, KCalCore::iTIPRequest);
    }


private slots:
    void initTestCase()
    {
        Sink::Test::initTest();

        auto account = ApplicationDomainType::createEntity<SinkAccount>();
        Sink::Store::create(account).exec().waitForFinished();

        auto resource = DummyResource::create(account.identifier());
        Sink::Store::create(resource).exec().waitForFinished();
        resourceId = resource.identifier();
    }

    void testAccept()
    {


        auto calendar = ApplicationDomainType::createEntity<Calendar>(resourceId);
        Sink::Store::create(calendar).exec().waitForFinished();

        const QByteArray uid{"uid1"};
        const auto ical = createInvitation(uid);

        {
            InvitationController controller;
            controller.loadICal(ical);

            controller.setCalendar(ApplicationDomainType::Ptr::create(calendar));

            QTRY_COMPARE(controller.getState(), InvitationController::Unknown);

            controller.acceptAction()->execute();
            QTRY_COMPARE(controller.getState(), InvitationController::Accepted);

            QTRY_COMPARE(Sink::Store::read<Event>(Sink::Query{}.filter<Event::Calendar>(calendar)).size(), 1);

            auto list = Sink::Store::read<Event>(Sink::Query{}.filter<Event::Calendar>(calendar));
            QCOMPARE(list.size(), 1);
            auto event = KCalCore::ICalFormat().readIncidence(list.first().getIcal()).dynamicCast<KCalCore::Event>();
            QVERIFY(event);
            QCOMPARE(event->uid(), uid);

            const auto attendee = event->attendeeByMail("attendee1@test.com");
            QVERIFY(attendee);
            QCOMPARE(attendee->status(), KCalCore::Attendee::Accepted);
        }

        {
            InvitationController controller;
            controller.loadICal(ical);
            QTRY_COMPARE(controller.getState(), InvitationController::Accepted);
            QTRY_COMPARE(controller.getUid(), uid);
        }
    }
};

QTEST_MAIN(InvitationControllerTest)
#include "invitationcontrollertest.moc"
