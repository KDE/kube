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

#include <KCalCore/ICalFormat>
#include <KCalCore/MemoryCalendar>
#include <KCalCore/Event>
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

void InvitationController::handleReply(KCalCore::Event::Ptr icalEvent)
{
    using namespace Sink;
    using namespace Sink::ApplicationDomain;

    setMethod(InvitationMethod::Reply);

    auto attendees = icalEvent->attendees();

    if (!attendees.isEmpty()) {
        auto attendee = attendees.first();
        if (attendee.status() == KCalCore::Attendee::Declined) {
            setState(InvitationState::Declined);
        } else if (attendee.status() == KCalCore::Attendee::Accepted) {
            setState(InvitationState::Accepted);
        } else {
            setState(InvitationState::Unknown);
        }
        setName(assembleEmailAddress(attendee.name(), attendee.email()));
    }

    populateFromEvent(*icalEvent);
    setStart(icalEvent->dtStart());
    setEnd(icalEvent->dtEnd());
    setUid(icalEvent->uid().toUtf8());
}

void InvitationController::handleRequest(KCalCore::Event::Ptr icalEvent)
{
    using namespace Sink;
    using namespace Sink::ApplicationDomain;

    setMethod(InvitationMethod::Request);

    Query query;
    query.request<Event::Uid>();
    query.request<Event::Ical>();
    query.filter<Event::Uid>(icalEvent->uid().toUtf8());
    Store::fetchAll<Event>(query).then([this, icalEvent](const QList<Event::Ptr> &events) {
        if (!events.isEmpty()) {
            //Find the matching occurrence in case of exceptions
            const auto [event, localEvent] = [&] {
                for (const auto &e : events) {
                    auto ical = KCalCore::ICalFormat().readIncidence(e->getIcal()).dynamicCast<KCalCore::Event>();
                    if (ical && ical->instanceIdentifier() == icalEvent->instanceIdentifier()) {
                        return std::pair(*e, ical);
                    }
                }
                return std::pair<Event, KCalCore::Event::Ptr>{};
            }();
            if(!localEvent) {
                SinkWarning() << "Invalid ICal to process, ignoring...";
                return KAsync::null();
            }

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
            setEventState(InvitationController::New);
            //We don't even have a local copy, this is a new event
            populateFromEvent(*icalEvent);
            setStart(icalEvent->dtStart());
            setEnd(icalEvent->dtEnd());
            setUid(icalEvent->uid().toUtf8());
        }

        Query query;
        query.request<ApplicationDomain::Identity::Name>()
            .request<ApplicationDomain::Identity::Address>()
            .request<ApplicationDomain::Identity::Account>();
        auto job = Store::fetchAll<ApplicationDomain::Identity>(query)
            .guard(this)
            .then([this] (const QList<Identity::Ptr> &list) {
                if (list.isEmpty()) {
                    SinkWarning() << "Failed to find an identity";
                }
                for (const auto &identity : list) {
                    const auto id = attendeesController()->findByProperty("email", identity->getAddress());
                    if (!id.isEmpty()) {
                        const auto status = attendeesController()->value(id, "status").value<EventController::ParticipantStatus>();
                        if (status == EventController::Accepted) {
                            setState(InvitationController::Accepted);
                        } else if (status == EventController::Declined) {
                            setState(InvitationController::Declined);
                        } else {
                            setState(InvitationController::Unknown);
                        }
                        return;
                    } else {
                        SinkLog() << "No attendee found for " << identity->getAddress();
                    }
                }
                SinkWarning() << "Failed to find matching identity in list of attendees.";
                setState(InvitationState::NoMatch);
            });
        return job;
    }).exec();
}

void InvitationController::loadICal(const QString &ical)
{
    using namespace Sink;
    using namespace Sink::ApplicationDomain;

    KCalCore::Calendar::Ptr calendar(new KCalCore::MemoryCalendar{QTimeZone::systemTimeZone()});
    auto msg = KCalCore::ICalFormat{}.parseScheduleMessage(calendar, ical.toUtf8());
    if (!msg) {
        SinkWarning() << "Invalid schedule message to process, ignoring...";
        return;
    }
    auto icalEvent = msg->event().dynamicCast<KCalCore::Event>();
    if(!icalEvent) {
        SinkWarning() << "Invalid ICal to process, ignoring...";
        return;
    }

    mLoadedIcalEvent = icalEvent;

    switch (msg->method()) {
        case KCalCore::iTIPRequest:
            handleRequest(icalEvent);
            break;
        case KCalCore::iTIPReply:
            handleReply(icalEvent);
            break;
        default:
            SinkWarning() << "Invalid method " << msg->method();
    }

}

static void sendIMipReply(const QByteArray &accountId, const QString &from, const QString &fromName, KCalCore::Event::Ptr event, KCalCore::Attendee::PartStat status)
{
    const auto organizerEmail = event->organizer().fullName();

    if (organizerEmail.isEmpty()) {
        SinkWarning() << "Failed to find the organizer to send the reply to " << organizerEmail;
        return;
    }

    auto reply = KCalCore::Event::Ptr::create(*event);
    reply->clearAttendees();
    reply->addAttendee(KCalCore::Attendee(fromName, from, false, status));

    QString body;
    if (status == KCalCore::Attendee::Accepted) {
        body.append(QObject::tr("%1 has accepted the invitation to the following event").arg(fromName));
    } else {
        body.append(QObject::tr("%1 has declined the invitation to the following event").arg(fromName));
    }
    body.append("\n\n");
    body.append(EventController::eventToBody(*reply));

    QString subject;
    if (status == KCalCore::Attendee::Accepted) {
        subject = QObject::tr("\"%1\" has been accepted by %2").arg(event->summary()).arg(fromName);
    } else {
        subject = QObject::tr("\"%1\" has been declined by %2").arg(event->summary()).arg(fromName);
    }

    const auto msg = MailTemplates::createIMipMessage(
        from,
        {{organizerEmail}, {}, {}},
        subject,
        body,
        KCalCore::ICalFormat{}.createScheduleMessage(reply, KCalCore::iTIPReply)
    );

    SinkTrace() << "Msg " << msg->encodedContent();

    SinkUtils::sendMail(msg->encodedContent(true), accountId)
        .then([&] (const KAsync::Error &error) {
            if (error) {
                SinkWarning() << "Failed to send message " << error;
            }
        }).exec();
}

void InvitationController::storeEvent(InvitationState status)
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

            sendIMipReply(accountId, fromAddress, fromName, calcoreEvent, status == InvitationController::Accepted ? KCalCore::Attendee::Accepted : KCalCore::Attendee::Declined);

            if (getEventState() == InvitationController::New || getRecurrenceId().isValid()) {
                const auto calendar = getCalendar();
                if (!calendar) {
                    SinkWarning() << "No calendar selected";
                    return KAsync::error("No calendar selected");
                }

                Event event(calendar->resourceInstanceIdentifier());
                event.setIcal(KCalCore::ICalFormat().toICalString(calcoreEvent).toUtf8());
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
                event.setIcal(KCalCore::ICalFormat().toICalString(calcoreEvent).toUtf8());

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
    storeEvent(InvitationState::Accepted);
}

void InvitationController::decline()
{
    storeEvent(InvitationState::Declined);
}

