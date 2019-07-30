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

void InvitationController::loadICal(const QString &ical)
{
    using namespace Sink;
    using namespace Sink::ApplicationDomain;

    KCalCore::Calendar::Ptr calendar(new KCalCore::MemoryCalendar{QTimeZone::systemTimeZone()});
    auto msg = KCalCore::ICalFormat{}.parseScheduleMessage(calendar, ical.toUtf8());
    if (!msg) {
        SinkWarning() << "Invalid scheudle message to process, ignoring...";
        return;
    }
    auto icalEvent = msg->event().dynamicCast<KCalCore::Event>();
    if (msg->method() != KCalCore::iTIPRequest) {
        SinkWarning() << "Invalid method " << msg->method();
        return;
    }

    if(!icalEvent) {
        SinkWarning() << "Invalid ICal to process, ignoring...";
        return;
    }

    Query query;
    query.request<Event::Uid>();
    query.request<Event::Ical>();
    query.filter<Event::Uid>(icalEvent->uid().toUtf8());
    Store::fetchAll<Event>(query).then([this, icalEvent](const QList<Event::Ptr> &events) {
        if (!events.isEmpty()) {
            setState(InvitationState::Accepted);

            auto icalEvent = KCalCore::ICalFormat().readIncidence(events.first()->getIcal()).dynamicCast<KCalCore::Event>();
            if(!icalEvent) {
                SinkWarning() << "Invalid ICal to process, ignoring...";
                return;
            }
            populateFromEvent(*icalEvent);
            setStart(icalEvent->dtStart());
            setEnd(icalEvent->dtEnd());
            setUid(icalEvent->uid().toUtf8());
        } else {
            setState(InvitationState::Unknown);
            populateFromEvent(*icalEvent);
            setStart(icalEvent->dtStart());
            setEnd(icalEvent->dtEnd());
            setUid(icalEvent->uid().toUtf8());
        }
    }).exec();
}

static void sendIMipMessage(const QByteArray &accountId, const QString &from, const QString &fromName, KCalCore::Event::Ptr event)
{
    const auto organizerEmail = event->organizer()->fullName();

    if (organizerEmail.isEmpty()) {
        SinkWarning() << "Failed to find the organizer to send the reply to " << organizerEmail;
        return;
    }

    QString body;
    body.append(QObject::tr("%1 has accepted the invitation to the following event").arg(fromName));
    body.append("\n\n");
    body.append(EventController::eventToBody(*event));

    const auto msg = MailTemplates::createIMipMessage(
        from,
        {{organizerEmail}, {}, {}},
        QObject::tr("\"%1\" has been accepted by %2").arg(event->summary()).arg(fromName),
        body,
        KCalCore::ICalFormat{}.createScheduleMessage(event, KCalCore::iTIPReply)
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

    const auto calendar = getCalendar();
    if (!calendar) {
        qWarning() << "No calendar selected";
        return;
    }

    Query query;
    query.request<ApplicationDomain::Identity::Name>()
        .request<ApplicationDomain::Identity::Address>()
        .request<ApplicationDomain::Identity::Account>();
    auto job = Store::fetchAll<ApplicationDomain::Identity>(query)
        .then([=] (const QList<Identity::Ptr> &list) {
            if (list.isEmpty()) {
                qWarning() << "Failed to find an identity";
            }
            QString fromAddress;
            QString fromName;
            QByteArray accountId;
            bool foundMatch = false;
            for (const auto &identity : list) {
                const auto id = attendeesController()->findByProperty("email", identity->getAddress());
                if (!id.isEmpty()) {
                    if (status == InvitationController::Accepted) {
                        attendeesController()->setValue(id, "status", EventController::Accepted);
                    } else {
                        attendeesController()->setValue(id, "status", EventController::Declined);
                    }
                    fromAddress = identity->getAddress();
                    fromName = identity->getName();
                    accountId = identity->getAccount();
                    foundMatch = true;
                } else {
                    SinkLog() << "No identity found for " << identity->getAddress();
                }
            }
            if (!foundMatch) {
                qWarning() << "Failed to find a matching identity.";
                return KAsync::error("Failed to find a matching identity");
            }
            auto calcoreEvent = QSharedPointer<KCalCore::Event>::create();
            calcoreEvent->setUid(getUid());
            saveToEvent(*calcoreEvent);

            sendIMipMessage(accountId, fromAddress, fromName, calcoreEvent);


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

