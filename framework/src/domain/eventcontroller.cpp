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

#include "eventcontroller.h"

#include <sink/applicationdomaintype.h>
#include <sink/store.h>
#include <sink/log.h>

#include <KCalCore/ICalFormat>
#include <KCalCore/Event>
#include <QUuid>

#include "eventoccurrencemodel.h"

using namespace Sink::ApplicationDomain;

EventController::EventController()
    : Kube::Controller(),
    action_save{new Kube::ControllerAction{this, &EventController::save}}
{
    updateSaveAction();
}

void EventController::save()
{
    using namespace Sink;
    using namespace Sink::ApplicationDomain;

    const auto calendar = getCalendar();
    if (!calendar) {
        qWarning() << "No calendar selected";
        return;
    }

    const auto occurrenceVariant = getEventOccurrence();
    if (occurrenceVariant.isValid()) {
        const auto occurrence = occurrenceVariant.value<EventOccurrenceModel::Occurrence>();

        Sink::ApplicationDomain::Event event = *occurrence.domainObject;

        //Apply the changed properties on top of what's existing
        auto calcoreEvent = KCalCore::ICalFormat().readIncidence(event.getIcal()).dynamicCast<KCalCore::Event>();
        if(!calcoreEvent) {
            SinkWarning() << "Invalid ICal to process, ignoring...";
            return;
        }

        calcoreEvent->setSummary(getSummary());
        calcoreEvent->setDescription(getDescription());
        calcoreEvent->setDtStart(getStart());
        calcoreEvent->setDtEnd(getEnd());
        calcoreEvent->setAllDay(getAllDay());

        event.setIcal(KCalCore::ICalFormat().toICalString(calcoreEvent).toUtf8());
        event.setCalendar(*calendar);

        auto job = Store::modify(event)
            .then([&] (const KAsync::Error &error) {
                if (error) {
                    SinkWarning() << "Failed to save the event: " << error;
                }
                emit done();
            });

        run(job);
    } else {
        Sink::ApplicationDomain::Event event(calendar->resourceInstanceIdentifier());

        auto calcoreEvent = QSharedPointer<KCalCore::Event>::create();
        calcoreEvent->setUid(QUuid::createUuid().toString());
        calcoreEvent->setSummary(getSummary());
        calcoreEvent->setDescription(getDescription());
        calcoreEvent->setLocation(getLocation());
        calcoreEvent->setDtStart(getStart());
        calcoreEvent->setDtEnd(getEnd());
        calcoreEvent->setAllDay(getAllDay());

        event.setIcal(KCalCore::ICalFormat().toICalString(calcoreEvent).toUtf8());
        event.setCalendar(*calendar);

        auto job = Store::create(event)
            .then([&] (const KAsync::Error &error) {
                if (error) {
                    SinkWarning() << "Failed to save the event: " << error;
                }
                emit done();
            });

        run(job);
    }
}

void EventController::updateSaveAction()
{
    saveAction()->setEnabled(!getSummary().isEmpty());
}

void EventController::populateFromEvent(const KCalCore::Event &event)
{
    setSummary(event.summary());
    setDescription(event.description());
    setLocation(event.location());
    setRecurring(event.recurs());
    //TODO translate recurrence to string (e.g. weekly)
    setRecurrenceString("");
    setAllDay(event.allDay());
}

void EventController::init()
{
    using namespace Sink;

    const auto occurrenceVariant = getEventOccurrence();
    if (occurrenceVariant.isValid()) {
        const auto occurrence = occurrenceVariant.value<EventOccurrenceModel::Occurrence>();

        Sink::ApplicationDomain::Event event = *occurrence.domainObject;


        setCalendar(ApplicationDomainType::Ptr::create(ApplicationDomainType::createEntity<ApplicationDomain::Calendar>(event.resourceInstanceIdentifier(), event.getCalendar())));

        auto icalEvent = KCalCore::ICalFormat().readIncidence(event.getIcal()).dynamicCast<KCalCore::Event>();
        if(!icalEvent) {
            SinkWarning() << "Invalid ICal to process, ignoring...";
            return;
        }
        populateFromEvent(*icalEvent);
        setStart(occurrence.start);
        setEnd(occurrence.end);
    }
}

void EventController::remove()
{
    const auto occurrenceVariant = getEventOccurrence();
    if (occurrenceVariant.isValid()) {
        const auto occurrence = occurrenceVariant.value<EventOccurrenceModel::Occurrence>();
        Sink::ApplicationDomain::Event event = *occurrence.domainObject;
        run(Sink::Store::remove(event));
    }
}
