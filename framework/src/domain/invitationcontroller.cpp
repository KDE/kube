/*
 *  Copyright (C) 2017 Michael Bohlender, <michael.bohlender@kdemail.net>
 *  Copyright (C) 2018 Christian Mollekopf, <mollekopf@kolabsys.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "invitationcontroller.h"

#include <sink/applicationdomaintype.h>
#include <sink/store.h>
#include <sink/log.h>

#include <KCalendarCore/ICalFormat>
#include <KCalendarCore/MemoryCalendar>
#include <KCalendarCore/Event>
#include <QUuid>

#include "mailtemplates.h"
#include "sinkutils.h"

using namespace Sink::ApplicationDomain;

InvitationController::InvitationController()
    : EventController(),
    action_accept{new Kube::ControllerAction{this, &InvitationController::accept}},
    action_decline{new Kube::ControllerAction{this, &InvitationController::decline}}
{
}

static QString assembleEmailAddress(const QString &name, const QString &email) {
    KMime::Types::Mailbox mb;
    mb.setName(name);
    mb.setAddress(email.toUtf8());
    return mb.prettyAddress();
}

static KAsync::Job<std::pair<Sink::ApplicationDomain::Event, KCalendarCore::Event::Ptr>> findExistingEvents(const QByteArray &uid, const QString &instanceIdentifier)
{
    using namespace Sink;
    using namespace Sink::ApplicationDomain;
    Query query;
    query.request<Event::Uid>();
    query.request<Event::Ical>();
    query.filter<Event::Uid>(uid);
    return Store::fetchAll<Event>(query).then([=](const QList<Event::Ptr> &events) {
        //Find the matching occurrence in case of exceptions
        for (const auto &e : events) {
            auto ical = KCalendarCore::ICalFormat().readIncidence(e->getIcal()).dynamicCast<KCalendarCore::Event>();
            if (ical && ical->instanceIdentifier() == instanceIdentifier) {
                return std::pair(*e, ical);
            }
        }
        return std::pair<Event, KCalendarCore::Event::Ptr>{};
    });
}

void InvitationController::handleReply(KCalendarCore::Event::Ptr icalEvent)
{
    using namespace Sink;
    using namespace Sink::ApplicationDomain;

    setMethod(InvitationMethod::Reply);

    auto attendees = icalEvent->attendees();

    if (!attendees.isEmpty()) {
        auto attendee = attendees.first();
        if (attendee.status() == KCalendarCore::Attendee::Declined) {
            setState(ParticipantStatus::Declined);
        } else if (attendee.status() == KCalendarCore::Attendee::Accepted) {
            setState(ParticipantStatus::Accepted);
        } else {
            setState(ParticipantStatus::Unknown);
        }
        setName(assembleEmailAddress(attendee.name(), attendee.email()));
    }

    populateFromEvent(*icalEvent);
    setStart(icalEvent->dtStart());
    setEnd(icalEvent->dtEnd());
    setUid(icalEvent->uid().toUtf8());
}

void InvitationController::handleCancellation(KCalendarCore::Event::Ptr icalEvent)
{
    using namespace Sink;
    using namespace Sink::ApplicationDomain;

    setMethod(InvitationMethod::Cancel);
    setState(InvitationController::Cancelled);

    findExistingEvents(icalEvent->uid().toUtf8(), icalEvent->instanceIdentifier())
    .then([this, icalEvent](const std::pair<Event, KCalendarCore::Event::Ptr> &pair) {
        const auto [event, localEvent] = pair;
        if (localEvent) {
            mExistingEvent = event;
            if (icalEvent->revision() > localEvent->revision()) {
                setEventState(InvitationController::Update);
            } else {
                setEventState(InvitationController::Existing);
            }
        } else {
            //Already removed the event?
            setEventState(InvitationController::Existing);
        }

        if (icalEvent->recurrenceId().isValid()) {
            setRecurrenceId(icalEvent->recurrenceId());
        }

        populateFromEvent(*icalEvent);
        setStart(icalEvent->dtStart());
        setEnd(icalEvent->dtEnd());
        setUid(icalEvent->uid().toUtf8());
    }).exec();

}

KAsync::Job<EventController::ParticipantStatus> InvitationController::findAttendeeStatus()
{
    using namespace Sink;
    using namespace Sink::ApplicationDomain;

    Query query;
    query.request<ApplicationDomain::Identity::Name>()
        .request<ApplicationDomain::Identity::Address>()
        .request<ApplicationDomain::Identity::Account>();
    auto job = Store::fetchAll<ApplicationDomain::Identity>(query)
        .then([this] (const QList<Identity::Ptr> &list) {
            if (list.isEmpty()) {
                SinkWarning() << "Failed to find an identity";
            }
            for (const auto &identity : list) {
                const auto id = attendeesController()->findByProperty("email", identity->getAddress());
                if (!id.isEmpty()) {
                    return attendeesController()->value(id, "status").value<EventController::ParticipantStatus>();
                } else {
                    SinkLog() << "No attendee found for " << identity->getAddress();
                }
            }
            SinkWarning() << "Failed to find matching identity in list of attendees.";
            return EventController::NoMatch;
        });
    return job;
}

void InvitationController::handleRequest(KCalendarCore::Event::Ptr icalEvent)
{
    using namespace Sink;
    using namespace Sink::ApplicationDomain;

    setMethod(InvitationMethod::Request);

    findExistingEvents(icalEvent->uid().toUtf8(), icalEvent->instanceIdentifier())
    .then([this, icalEvent](const std::pair<Event, KCalendarCore::Event::Ptr> &pair) {
        const auto [event, localEvent] = pair;
        if (localEvent) {
            mExistingEvent = event;
            if (icalEvent->revision() > localEvent->revision()) {
                setEventState(InvitationController::Update);
                //The invitation is more recent, this is an update to an existing event
                populateFromEvent(*icalEvent);
                if (icalEvent->recurrenceId().isValid()) {
                    setRecurrenceId(icalEvent->recurrenceId());
                }
                setStart(icalEvent->dtStart());
                setEnd(icalEvent->dtEnd());
                setUid(icalEvent->uid().toUtf8());
            } else {
                setEventState(InvitationController::Existing);
                //Our local copy is more recent (we probably already dealt with the invitation)
                populateFromEvent(*localEvent);
                setStart(localEvent->dtStart());
                setEnd(localEvent->dtEnd());
                setUid(localEvent->uid().toUtf8());
            }
        } else {
            mExistingEvent = {};
            if (icalEvent->recurrenceId().isValid()) {
                setRecurrenceId(icalEvent->recurrenceId());
                setEventState(InvitationController::Update);
            } else {
                setEventState(InvitationController::New);
            }
            //We don't even have a local copy, this is a new event
            populateFromEvent(*icalEvent);
            setStart(icalEvent->dtStart());
            setEnd(icalEvent->dtEnd());
            setUid(icalEvent->uid().toUtf8());
        }

        return findAttendeeStatus()
            .guard(this)
            .then([this] (ParticipantStatus status) {
                setState(status);
            });

    }).exec();
}

void InvitationController::loadICal(const QString &ical)
{
    using namespace Sink;
    using namespace Sink::ApplicationDomain;

    KCalendarCore::Calendar::Ptr calendar(new KCalendarCore::MemoryCalendar{QTimeZone::systemTimeZone()});
    auto msg = KCalendarCore::ICalFormat{}.parseScheduleMessage(calendar, ical.toUtf8());
    if (!msg) {
        SinkWarning() << "Invalid schedule message to process, ignoring...";
        return;
    }
    auto icalEvent = msg->event().dynamicCast<KCalendarCore::Event>();
    if(!icalEvent) {
        SinkWarning() << "Invalid ICal to process, ignoring...";
        return;
    }

    mLoadedIcalEvent = icalEvent;

    switch (msg->method()) {
        case KCalendarCore::iTIPRequest:
            //Roundcube sends cancellations not as METHOD=CANCEL, but instead updates the event status.
            if (icalEvent->status() == KCalendarCore::Incidence::StatusCanceled) {
                handleCancellation(icalEvent);
                break;
            }

            handleRequest(icalEvent);
            break;
        case KCalendarCore::iTIPReply:
            handleReply(icalEvent);
            break;
        case KCalendarCore::iTIPCancel:
            handleCancellation(icalEvent);
            break;
        default:
            SinkWarning() << "Invalid method " << msg->method();
    }

}

static void sendIMipReply(const QByteArray &accountId, const QString &from, const QString &fromName, KCalendarCore::Event::Ptr event, KCalendarCore::Attendee::PartStat status)
{
    const auto organizerEmail = event->organizer().fullName();

    if (organizerEmail.isEmpty()) {
        SinkWarning() << "Failed to find the organizer to send the reply to " << organizerEmail;
        return;
    }

    auto reply = KCalendarCore::Event::Ptr::create(*event);
    reply->clearAttendees();
    reply->addAttendee(KCalendarCore::Attendee(fromName, from, false, status));

    QString body;
    if (status == KCalendarCore::Attendee::Accepted) {
        body.append(QObject::tr("%1 has accepted the invitation to the following event").arg(fromName));
    } else {
        body.append(QObject::tr("%1 has declined the invitation to the following event").arg(fromName));
    }
    body.append("\n\n");
    body.append(EventController::eventToBody(*reply));

    QString subject;
    if (status == KCalendarCore::Attendee::Accepted) {
        subject = QObject::tr("\"%1\" has been accepted by %2").arg(event->summary()).arg(fromName);
    } else {
        subject = QObject::tr("\"%1\" has been declined by %2").arg(event->summary()).arg(fromName);
    }

    const auto msg = MailTemplates::createIMipMessage(
        from,
        {{organizerEmail}, {}, {}},
        subject,
        body,
        KCalendarCore::ICalFormat{}.createScheduleMessage(reply, KCalendarCore::iTIPReply)
    );

    SinkTrace() << "Msg " << msg->encodedContent();

    SinkUtils::sendMail(msg->encodedContent(true), accountId)
        .then([&] (const KAsync::Error &error) {
            if (error) {
                SinkWarning() << "Failed to send message " << error;
            }
        }).exec();
}

void InvitationController::storeEvent(ParticipantStatus status)
{
    using namespace Sink;
    using namespace Sink::ApplicationDomain;

    Query query;
    query.request<ApplicationDomain::Identity::Name>()
        .request<ApplicationDomain::Identity::Address>()
        .request<ApplicationDomain::Identity::Account>();
    auto job = Store::fetchAll<ApplicationDomain::Identity>(query)
        .guard(this)
        .then([this, status] (const QList<Identity::Ptr> &list) {
            if (list.isEmpty()) {
                SinkWarning() << "Failed to find an identity";
            }
            QString fromAddress;
            QString fromName;
            QByteArray accountId;
            bool foundMatch = false;
            for (const auto &identity : list) {
                const auto id = attendeesController()->findByProperty("email", identity->getAddress());
                if (!id.isEmpty()) {
                    auto participantStatus = status == InvitationController::Accepted ? EventController::Accepted : EventController::Declined;
                    attendeesController()->setValue(id, "status", participantStatus);
                    fromAddress = identity->getAddress();
                    fromName = identity->getName();
                    accountId = identity->getAccount();
                    foundMatch = true;
                } else {
                    SinkLog() << "No identity found for " << identity->getAddress();
                }
            }
            if (!foundMatch) {
                SinkWarning() << "Failed to find a matching identity.";
                return KAsync::error("Failed to find a matching identity");
            }

            auto calcoreEvent = mLoadedIcalEvent;
            calcoreEvent->setUid(getUid());
            saveToEvent(*calcoreEvent);

            sendIMipReply(accountId, fromAddress, fromName, calcoreEvent, status == InvitationController::Accepted ? KCalendarCore::Attendee::Accepted : KCalendarCore::Attendee::Declined);

            if (mExistingEvent.identifier().isEmpty()) {
                const auto calendar = getCalendar();
                if (!calendar) {
                    SinkWarning() << "No calendar selected";
                    return KAsync::error("No calendar selected");
                }

                Event event(calendar->resourceInstanceIdentifier());
                event.setIcal(KCalendarCore::ICalFormat().toICalString(calcoreEvent).toUtf8());
                event.setCalendar(*calendar);

                return Store::create(event)
                    .then([=] (const KAsync::Error &error) {
                        if (error) {
                            SinkWarning() << "Failed to save the event: " << error;
                        }
                        setState(status);
                        emit done();
                    });
            } else {
                Event event(mExistingEvent);
                event.setIcal(KCalendarCore::ICalFormat().toICalString(calcoreEvent).toUtf8());

                return Store::modify(event)
                    .then([=] (const KAsync::Error &error) {
                        if (error) {
                            SinkWarning() << "Failed to update the event: " << error;
                        }
                        setState(status);
                        setEventState(InvitationController::Existing);
                        emit done();
                    });
            }
        });

    run(job);
}

void InvitationController::accept()
{
    if (getMethod() == Cancel) {
        storeEvent(ParticipantStatus::Cancelled);
    } else {
        storeEvent(ParticipantStatus::Accepted);
    }
}

void InvitationController::decline()
{
    storeEvent(ParticipantStatus::Declined);
}

