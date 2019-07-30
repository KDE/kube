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

#include "controller.h"
#include "completer.h"
#include "selector.h"

namespace KCalCore {
    class Event;
};

class KUBE_EXPORT EventController : public Kube::Controller
{
    Q_OBJECT

    // Input properties
    KUBE_CONTROLLER_PROPERTY(QVariant, EventOccurrence, eventOccurrence)

    //Interface properties
    KUBE_CONTROLLER_PROPERTY(QByteArray, AccountId, accountId)
    KUBE_CONTROLLER_PROPERTY(QString, Summary, summary)
    KUBE_CONTROLLER_PROPERTY(QString, Description, description)
    KUBE_CONTROLLER_PROPERTY(QString, Location, location)
    KUBE_CONTROLLER_PROPERTY(QDateTime, Start, start)
    KUBE_CONTROLLER_PROPERTY(QDateTime, End, end)
    KUBE_CONTROLLER_PROPERTY(QString, RecurrenceString, recurrenceString)
    KUBE_CONTROLLER_PROPERTY(bool, AllDay, allDay)
    KUBE_CONTROLLER_PROPERTY(bool, Recurring, recurring)
    KUBE_CONTROLLER_PROPERTY(Sink::ApplicationDomain::ApplicationDomainType::Ptr, Calendar, calendar)

    KUBE_CONTROLLER_PROPERTY(QString, Organizer, organizer)
    KUBE_CONTROLLER_LISTCONTROLLER(attendees)

    Q_PROPERTY (Completer* attendeeCompleter READ attendeeCompleter CONSTANT)
    Q_PROPERTY (Selector* identitySelector READ identitySelector CONSTANT)

    KUBE_CONTROLLER_ACTION(save)

public:
    enum ParticipantStatus {
        Unknown,
        Accepted,
        Declined,
    };
    Q_ENUM(ParticipantStatus);

    explicit EventController();

    void init() override;
    Q_INVOKABLE void remove();

    Completer *attendeeCompleter() const;
    Selector *identitySelector() const;

    static QString eventToBody(const KCalCore::Event &event);

protected:
    void populateFromEvent(const KCalCore::Event &event);
    void saveToEvent(KCalCore::Event &event);

private slots:
    void updateSaveAction();

private:
    QScopedPointer<Completer> mAttendeeCompleter;
    QScopedPointer<Selector> mIdentitySelector;
};
