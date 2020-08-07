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


#pragma once
#include "kube_export.h"

#include <QString>
#include <QDateTime>

#include <sink/applicationdomaintype.h>

#include "eventcontroller.h"

#include <KCalCore/Event>
class KUBE_EXPORT InvitationController : public EventController
{
    Q_OBJECT

public:
    enum InvitationState {
        Unknown,
        Accepted,
        Declined,
        Cancelled, //This is not part of the usual partstat in ical, but for our purposes where it belongs
        NoMatch
    };
    Q_ENUM(InvitationState);

    enum InvitationMethod {
        Reply,
        Request,
        Cancel
    };
    Q_ENUM(InvitationMethod);

    enum InvitationEventState {
        New,
        Existing,
        Update
    };
    Q_ENUM(InvitationEventState);

    KUBE_CONTROLLER_PROPERTY(QByteArray, Uid, uid)
    KUBE_CONTROLLER_PROPERTY(InvitationState, State, state)
    KUBE_CONTROLLER_PROPERTY(InvitationMethod, Method, method)
    KUBE_CONTROLLER_PROPERTY(InvitationEventState, EventState, eventState)
    KUBE_CONTROLLER_PROPERTY(QString, Name, name)
    KUBE_CONTROLLER_PROPERTY(QDateTime, RecurrenceId, recurrenceId)

    KUBE_CONTROLLER_ACTION(accept)
    KUBE_CONTROLLER_ACTION(decline)

public:
    explicit InvitationController();

    Q_INVOKABLE void loadICal(const QString &message);

private:
    void handleRequest(KCalCore::Event::Ptr icalEvent);
    void handleReply(KCalCore::Event::Ptr icalEvent);
    void handleCancellation(KCalCore::Event::Ptr icalEvent);
    void storeEvent(InvitationState);
    Sink::ApplicationDomain::Event mExistingEvent;
    KCalCore::Event::Ptr mLoadedIcalEvent;
};
