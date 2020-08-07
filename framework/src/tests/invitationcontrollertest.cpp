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

    QString createInvitation(const QByteArray &uid, const QString &summary, int revision, QDateTime dtStart = QDateTime::currentDateTime(), bool recurring = false, QDateTime recurrenceId = {}, bool cancelled = false, KCalCore::iTIPMethod method = KCalCore::iTIPRequest)
    {
        auto calcoreEvent = QSharedPointer<KCalCore::Event>::create();
        calcoreEvent->setUid(uid);
        calcoreEvent->setSummary(summary);
        calcoreEvent->setDescription("description");
        calcoreEvent->setLocation("location");
        calcoreEvent->setDtStart(dtStart);
        calcoreEvent->setOrganizer("organizer@test.com");
        calcoreEvent->addAttendee(KCalCore::Attendee("John Doe", "attendee1@test.com", true, KCalCore::Attendee::NeedsAction));
        calcoreEvent->setRevision(revision);
        if (cancelled) {
            calcoreEvent->setStatus(KCalCore::Incidence::StatusCanceled);
        }

        if (recurring) {
            calcoreEvent->recurrence()->setDaily(1);
        }
        if (recurrenceId.isValid()) {
            calcoreEvent->setRecurrenceId(recurrenceId);
        }

        return KCalCore::ICalFormat{}.createScheduleMessage(calcoreEvent, method);
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

            QTRY_COMPARE(controller.getMethod(), InvitationController::Request);
            QTRY_COMPARE(controller.getState(), InvitationController::Unknown);
            QTRY_COMPARE(controller.getEventState(), InvitationController::New);

            controller.acceptAction()->execute();
            Sink::ResourceControl::flushMessageQueue(resourceId).exec().waitForFinished();
            QCOMPARE(controller.getState(), InvitationController::Accepted);

            //Ensure the event is stored
            QCOMPARE(Sink::Store::read<Event>(Sink::Query{}.filter<Event::Calendar>(calendar)).size(), 1);

            auto list = Sink::Store::read<Event>(Sink::Query{}.filter<Event::Calendar>(calendar));
            QCOMPARE(list.size(), 1);

            auto event = KCalCore::ICalFormat().readIncidence(list.first().getIcal()).dynamicCast<KCalCore::Event>();
            QVERIFY(event);
            QCOMPARE(event->uid().toUtf8(), uid);
            QCOMPARE(event->organizer().fullName(), QLatin1String{"organizer@test.com"});

            const auto attendee = event->attendeeByMail("attendee1@test.com");
            QCOMPARE(attendee.status(), KCalCore::Attendee::Accepted);

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
            Sink::ResourceControl::flushMessageQueue(resourceId).exec().waitForFinished();

            QCOMPARE(controller.getState(), InvitationController::Accepted);

            //Ensure the event is stored
            QCOMPARE(Sink::Store::read<Event>(Sink::Query{}.filter<Event::Calendar>(calendar)).size(), 1);

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

    void testAcceptRecurrenceException()
    {
        auto calendar = ApplicationDomainType::createEntity<Calendar>(resourceId);
        Sink::Store::create(calendar).exec().waitForFinished();

        const QByteArray uid{"uid2"};
        auto dtstart = QDateTime{{2020, 1, 1}, {14, 0, 0}, Qt::UTC};
        const auto ical = createInvitation(uid, "summary", 0, dtstart, true);

        {
            InvitationController controller;
            controller.loadICal(ical);

            controller.setCalendar(ApplicationDomainType::Ptr::create(calendar));

            QTRY_COMPARE(controller.getState(), InvitationController::Unknown);
            QTRY_COMPARE(controller.getEventState(), InvitationController::New);

            controller.acceptAction()->execute();
            Sink::ResourceControl::flushMessageQueue(resourceId).exec().waitForFinished();
            QCOMPARE(controller.getState(), InvitationController::Accepted);

            //Ensure the event is stored
            QCOMPARE(Sink::Store::read<Event>(Sink::Query{}.filter<Event::Calendar>(calendar)).size(), 1);

            auto list = Sink::Store::read<Event>(Sink::Query{}.filter<Event::Calendar>(calendar));
            QCOMPARE(list.size(), 1);

            auto event = KCalCore::ICalFormat().readIncidence(list.first().getIcal()).dynamicCast<KCalCore::Event>();
            QVERIFY(event);
            QCOMPARE(event->uid().toUtf8(), uid);
            QCOMPARE(event->organizer().fullName(), QLatin1String{"organizer@test.com"});
        }

        //Reload the event
        {
            InvitationController controller;
            controller.loadICal(ical);
            QTRY_COMPARE(controller.getState(), InvitationController::Accepted);
            QTRY_COMPARE(controller.getUid(), uid);
            QVERIFY(!controller.getRecurrenceId().isValid());
        }

        //Load an exception and accept it
        {
            InvitationController controller;
            //TODO I suppose the revision of the exception can also be 0?
            controller.loadICal(createInvitation(uid, "exceptionSummary", 1, dtstart.addSecs(3600), false, dtstart));
            controller.setCalendar(ApplicationDomainType::Ptr::create(calendar));
            QTRY_COMPARE(controller.getEventState(), InvitationController::Update);
            QTRY_COMPARE(controller.getUid(), uid);
            QTRY_COMPARE(controller.getState(), InvitationController::Unknown);
            QTRY_COMPARE(controller.getRecurrenceId(), dtstart);

            //Accept the update
            controller.acceptAction()->execute();
            Sink::ResourceControl::flushMessageQueue(resourceId).exec().waitForFinished();

            QCOMPARE(controller.getState(), InvitationController::Accepted);

            //Ensure the event is stored
            QCOMPARE(Sink::Store::read<Event>(Sink::Query{}.filter<Event::Calendar>(calendar)).size(), 2);

            auto list = Sink::Store::read<Event>(Sink::Query{}.filter<Event::Calendar>(calendar));
            QCOMPARE(list.size(), 2);

            for (const auto &entry : list) {
                auto event = KCalCore::ICalFormat().readIncidence(entry.getIcal()).dynamicCast<KCalCore::Event>();
                QVERIFY(event);
                QCOMPARE(event->uid().toUtf8(), uid);
                if (event->recurrenceId().isValid()) {
                    QCOMPARE(event->summary(), QLatin1String{"exceptionSummary"});
                } else {
                    QCOMPARE(event->summary(), QLatin1String{"summary"});
                }
            }
        }

        //Update the exception and accept it
        {
            InvitationController controller;
            controller.loadICal(createInvitation(uid, "exceptionSummary2", 3, dtstart.addSecs(3600), false, dtstart));
            controller.setCalendar(ApplicationDomainType::Ptr::create(calendar));
            QTRY_COMPARE(controller.getEventState(), InvitationController::Update);
            QTRY_COMPARE(controller.getUid(), uid);
            QTRY_COMPARE(controller.getState(), InvitationController::Unknown);
            QTRY_COMPARE(controller.getRecurrenceId(), dtstart);

            //Accept the update
            controller.acceptAction()->execute();
            Sink::ResourceControl::flushMessageQueue(resourceId).exec().waitForFinished();

            QTRY_COMPARE(controller.getState(), InvitationController::Accepted);

            //Ensure the event is stored
            QCOMPARE(Sink::Store::read<Event>(Sink::Query{}.filter<Event::Calendar>(calendar)).size(), 2);

            auto list = Sink::Store::read<Event>(Sink::Query{}.filter<Event::Calendar>(calendar));
            QCOMPARE(list.size(), 2);

            for (const auto &entry : list) {
                auto event = KCalCore::ICalFormat().readIncidence(entry.getIcal()).dynamicCast<KCalCore::Event>();
                QVERIFY(event);
                QCOMPARE(event->uid().toUtf8(), uid);
                if (event->recurrenceId().isValid()) {
                    QCOMPARE(event->summary(), QLatin1String{"exceptionSummary2"});
                } else {
                    QCOMPARE(event->summary(), QLatin1String{"summary"});
                }
            }
        }

        //Update the main event and accept it
        {
            InvitationController controller;
            controller.loadICal(createInvitation(uid, "summary2", 4, dtstart, true));
            controller.setCalendar(ApplicationDomainType::Ptr::create(calendar));
            QTRY_COMPARE(controller.getEventState(), InvitationController::Update);
            QTRY_COMPARE(controller.getUid(), uid);
            QTRY_COMPARE(controller.getState(), InvitationController::Unknown);
            QVERIFY(!controller.getRecurrenceId().isValid());

            //Accept the update
            controller.acceptAction()->execute();
            Sink::ResourceControl::flushMessageQueue(resourceId).exec().waitForFinished();

            QTRY_COMPARE(controller.getState(), InvitationController::Accepted);

            //Ensure the event is stored
            QCOMPARE(Sink::Store::read<Event>(Sink::Query{}.filter<Event::Calendar>(calendar)).size(), 2);

            auto list = Sink::Store::read<Event>(Sink::Query{}.filter<Event::Calendar>(calendar));
            QCOMPARE(list.size(), 2);

            for (const auto &entry : list) {
                auto event = KCalCore::ICalFormat().readIncidence(entry.getIcal()).dynamicCast<KCalCore::Event>();
                QVERIFY(event);
                QCOMPARE(event->uid().toUtf8(), uid);
                if (event->recurrenceId().isValid()) {
                    QCOMPARE(event->summary(), QLatin1String{"exceptionSummary2"});
                } else {
                    QCOMPARE(event->summary(), QLatin1String{"summary2"});
                }
            }
        }
    }

    void testCancellation()
    {
        auto calendar = ApplicationDomainType::createEntity<Calendar>(resourceId);
        Sink::Store::create(calendar).exec().waitForFinished();

        const QByteArray uid{"uid3"};

        //Create event
        {
            InvitationController controller;
            const auto ical = createInvitation(uid, "summary", 0);
            controller.loadICal(ical);
            QTRY_COMPARE(controller.getMethod(), InvitationController::Request);
            QTRY_COMPARE(controller.getEventState(), InvitationController::New);
            controller.setCalendar(ApplicationDomainType::Ptr::create(calendar));
            controller.acceptAction()->execute();
            Sink::ResourceControl::flushMessageQueue(resourceId).exec().waitForFinished();
            QCOMPARE(Sink::Store::read<Event>(Sink::Query{}.filter<Event::Calendar>(calendar)).size(), 1);
        }

        //Cancellation via status update, like roundcube does
        {
            InvitationController controller;
            const auto ical = createInvitation(uid, "summary", 1, QDateTime::currentDateTime(), false, {}, true);
            controller.loadICal(ical);

            QTRY_COMPARE(controller.getMethod(), InvitationController::Request);
            QTRY_COMPARE(controller.getState(), InvitationController::Cancelled);
            QTRY_COMPARE(controller.getEventState(), InvitationController::Update);
        }

        //Cancellation per rfc
        {
            InvitationController controller;
            const auto ical = createInvitation(uid, "summary", 1, QDateTime::currentDateTime(), false, {}, false, KCalCore::iTIPCancel);
            controller.loadICal(ical);

            QTRY_COMPARE(controller.getMethod(), InvitationController::Cancel);
            QTRY_COMPARE(controller.getState(), InvitationController::Cancelled);
            QTRY_COMPARE(controller.getEventState(), InvitationController::Update);

            controller.acceptAction()->execute();
            Sink::ResourceControl::flushMessageQueue(resourceId).exec().waitForFinished();
            QTRY_COMPARE(controller.getState(), InvitationController::Cancelled);
            QCOMPARE(Sink::Store::read<Event>(Sink::Query{}.filter<Event::Calendar>(calendar)).size(), 0);
        }
    }

    void testReply()
    {
        auto calendar = ApplicationDomainType::createEntity<Calendar>(resourceId);
        Sink::Store::create(calendar).exec().waitForFinished();

        const QByteArray uid{"uid1"};
        const auto ical = createInvitation(uid, "summary", 0, QDateTime::currentDateTime(), false, {}, false, KCalCore::iTIPReply);

        {
            InvitationController controller;
            controller.loadICal(ical);

            controller.setCalendar(ApplicationDomainType::Ptr::create(calendar));

            QTRY_COMPARE(controller.getMethod(), InvitationController::Reply);
            QTRY_COMPARE(controller.getState(), InvitationController::Unknown);

            controller.acceptAction()->execute();
            Sink::ResourceControl::flushMessageQueue(resourceId).exec().waitForFinished();
            QCOMPARE(controller.getState(), InvitationController::Accepted);

            //Ensure the event is stored
            QCOMPARE(Sink::Store::read<Event>(Sink::Query{}.filter<Event::Calendar>(calendar)).size(), 1);

            auto list = Sink::Store::read<Event>(Sink::Query{}.filter<Event::Calendar>(calendar));
            QCOMPARE(list.size(), 1);
        }
    }

};

QTEST_MAIN(InvitationControllerTest)
#include "invitationcontrollertest.moc"
