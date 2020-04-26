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
#include <KMime/Message>
#include "invitationcontroller.h"

using namespace Sink::ApplicationDomain;


class InvitationControllerTest : public QObject
{
    Q_OBJECT

    QByteArray resourceId;
    QByteArray mailtransportResourceId;

    QString createInvitation(const QByteArray &uid, const QString &summary, int revision)
    {
        auto calcoreEvent = QSharedPointer<KCalCore::Event>::create();
        calcoreEvent->setUid(uid);
        calcoreEvent->setSummary(summary);
        calcoreEvent->setDescription("description");
        calcoreEvent->setLocation("location");
        calcoreEvent->setDtStart(QDateTime::currentDateTime());
        calcoreEvent->setOrganizer("organizer@test.com");
        calcoreEvent->addAttendee(KCalCore::Attendee::Ptr::create("John Doe", "attendee1@test.com", true, KCalCore::Attendee::NeedsAction));
        calcoreEvent->setRevision(revision);

        return KCalCore::ICalFormat{}.createScheduleMessage(calcoreEvent, KCalCore::iTIPRequest);
    }


private slots:
    void initTestCase()
    {
        Sink::Test::initTest();

        auto account = ApplicationDomainType::createEntity<SinkAccount>();
        Sink::Store::create(account).exec().waitForFinished();

        auto identity = ApplicationDomainType::createEntity<Identity>();
        identity.setAccount(account);
        identity.setAddress("attendee1@test.com");
        identity.setName("John Doe");

        Sink::Store::create(identity).exec().waitForFinished();

        auto resource = DummyResource::create(account.identifier());
        Sink::Store::create(resource).exec().waitForFinished();
        resourceId = resource.identifier();

        auto mailtransport = MailtransportResource::create(account.identifier());
        Sink::Store::create(mailtransport).exec().waitForFinished();
        mailtransportResourceId = mailtransport.identifier();
    }

    void testAccept()
    {
        auto calendar = ApplicationDomainType::createEntity<Calendar>(resourceId);
        Sink::Store::create(calendar).exec().waitForFinished();

        const QByteArray uid{"uid1"};
        const auto ical = createInvitation(uid, "summary", 0);

        {
            InvitationController controller;
            controller.loadICal(ical);

            controller.setCalendar(ApplicationDomainType::Ptr::create(calendar));

            QTRY_COMPARE(controller.getState(), InvitationController::Unknown);
            QTRY_COMPARE(controller.getEventState(), InvitationController::New);

            controller.acceptAction()->execute();
            QTRY_COMPARE(controller.getState(), InvitationController::Accepted);

            //Ensure the event is stored
            QTRY_COMPARE(Sink::Store::read<Event>(Sink::Query{}.filter<Event::Calendar>(calendar)).size(), 1);

            auto list = Sink::Store::read<Event>(Sink::Query{}.filter<Event::Calendar>(calendar));
            QCOMPARE(list.size(), 1);

            auto event = KCalCore::ICalFormat().readIncidence(list.first().getIcal()).dynamicCast<KCalCore::Event>();
            QVERIFY(event);
            QCOMPARE(event->uid().toUtf8(), uid);
            QCOMPARE(event->organizer()->fullName(), QLatin1String{"organizer@test.com"});

            const auto attendee = event->attendeeByMail("attendee1@test.com");
            QVERIFY(attendee);
            QCOMPARE(attendee->status(), KCalCore::Attendee::Accepted);

            //Ensure the mail is sent to the organizer
            QTRY_COMPARE(Sink::Store::read<Mail>(Sink::Query{}.resourceFilter(mailtransportResourceId)).size(), 1);
            auto mail = Sink::Store::read<Mail>(Sink::Query{}.resourceFilter(mailtransportResourceId)).first();
            auto msg = KMime::Message::Ptr(new KMime::Message);
            msg->setContent(mail.getMimeMessage());
            msg->parse();

            QCOMPARE(msg->to()->asUnicodeString(), QLatin1String{"organizer@test.com"});
            QCOMPARE(msg->from()->asUnicodeString(), QLatin1String{"attendee1@test.com"});
        }

        //Reload the event
        {
            InvitationController controller;
            controller.loadICal(ical);
            QTRY_COMPARE(controller.getState(), InvitationController::Accepted);
            QTRY_COMPARE(controller.getUid(), uid);
        }

        const auto updatedIcal = createInvitation(uid, "summary2", 1);
        //Load an update and accept it
        {
            InvitationController controller;
            controller.loadICal(updatedIcal);
            QTRY_COMPARE(controller.getEventState(), InvitationController::Update);
            QTRY_COMPARE(controller.getUid(), uid);

            //Accept the update
            controller.acceptAction()->execute();

            QTRY_COMPARE(controller.getState(), InvitationController::Accepted);

            //Ensure the event is stored
            QTRY_COMPARE(Sink::Store::read<Event>(Sink::Query{}.filter<Event::Calendar>(calendar)).size(), 1);

            auto list = Sink::Store::read<Event>(Sink::Query{}.filter<Event::Calendar>(calendar));
            QCOMPARE(list.size(), 1);

            auto event = KCalCore::ICalFormat().readIncidence(list.first().getIcal()).dynamicCast<KCalCore::Event>();
            QVERIFY(event);
            QCOMPARE(event->uid().toUtf8(), uid);
            QCOMPARE(event->summary(), QLatin1String{"summary2"});
        }

        //Reload the event
        {
            InvitationController controller;
            controller.loadICal(updatedIcal);
            QTRY_COMPARE(controller.getState(), InvitationController::Accepted);
            QTRY_COMPARE(controller.getUid(), uid);
            QCOMPARE(controller.getEventState(), InvitationController::Existing);
        }
    }
};

QTEST_MAIN(InvitationControllerTest)
#include "invitationcontrollertest.moc"
