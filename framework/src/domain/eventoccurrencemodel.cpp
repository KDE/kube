/*
    Copyright (c) 2018 Michael Bohlender <michael.bohlender@kdemail.net>
    Copyright (c) 2018 Christian Mollekopf <mollekopf@kolabsys.com>
    Copyright (c) 2018 Rémi Nicole <minijackson@riseup.net>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "eventoccurrencemodel.h"

#include <sink/log.h>
#include <sink/query.h>
#include <sink/store.h>
#include <sink/applicationdomaintype.h>

#include <QMetaEnum>

#include <KCalendarCore/ICalFormat>
#include <KCalendarCore/OccurrenceIterator>
#include <KCalendarCore/MemoryCalendar>

#include <entitycache.h>

#include <algorithm>
#include <vector>
#include <iterator>

using namespace Sink;

EventOccurrenceModel::EventOccurrenceModel(QObject *parent)
    : QAbstractItemModel(parent),
    mCalendarCache{EntityCache<ApplicationDomain::Calendar>::Ptr::create(QByteArrayList{{ApplicationDomain::Calendar::Color::name}})},
    mUpdateFromSourceDebouncer{100,[this] { this->updateFromSource(); }, this}
{
}

void EventOccurrenceModel::setStart(const QDate &start)
{
    if (start != mStart) {
        mStart = start;
        updateQuery();
    }
}

QDate EventOccurrenceModel::start() const
{
    return mStart;
}

void EventOccurrenceModel::setLength(int length)
{
    mLength = length;
    updateQuery();
}

int EventOccurrenceModel::length() const
{
    return mLength;
}

void EventOccurrenceModel::setCalendarFilter(const QList<QString> &calendarFilter)
{
    mCalendarFilter.clear();
    for (const auto &id : calendarFilter) {
        mCalendarFilter << id.toLatin1();
    }
    updateQuery();
}

void EventOccurrenceModel::setFilter(const QVariantMap &filter)
{
    mFilter = filter;
    updateQuery();
}

void EventOccurrenceModel::updateQuery()
{
    using namespace Sink::ApplicationDomain;
    mInitialItemsLoaded = false;
    if (mCalendarFilter.isEmpty() || !mLength || !mStart.isValid()) {
        mSourceModel.clear();
        refreshView();
        return;
    }
    mEnd = mStart.addDays(mLength);

    Sink::Query query;
    query.setFlags(Sink::Query::LiveQuery);
    query.request<Event::Summary>();
    query.request<Event::Description>();
    query.request<Event::StartTime>();
    query.request<Event::EndTime>();
    query.request<Event::Calendar>();
    query.request<Event::Ical>();
    query.request<Event::AllDay>();

    query.filter<Event::StartTime, Event::EndTime>(Sink::Query::Comparator(QVariantList{mStart, mEnd}, Sink::Query::Comparator::Overlap));

    query.setPostQueryFilter([=, filter = mFilter, calendarFilter = mCalendarFilter] (const ApplicationDomain::ApplicationDomainType &entity){
        const Sink::ApplicationDomain::Event event(entity);
        if (!calendarFilter.contains(event.getCalendar())) {
            return false;
        }

        for (auto it = filter.constBegin(); it!= filter.constEnd(); it++) {
            if (event.getProperty(it.key().toLatin1()) != it.value()) {
                return false;
            }
        }

        return true;
    });

    mSourceModel = Store::loadModel<ApplicationDomain::Event>(query);

    QObject::connect(mSourceModel.data(), &QAbstractItemModel::dataChanged, this, &EventOccurrenceModel::refreshView);
    QObject::connect(mSourceModel.data(), &QAbstractItemModel::layoutChanged, this, &EventOccurrenceModel::refreshView);
    QObject::connect(mSourceModel.data(), &QAbstractItemModel::modelReset, this, &EventOccurrenceModel::refreshView);
    QObject::connect(mSourceModel.data(), &QAbstractItemModel::rowsInserted, this, &EventOccurrenceModel::refreshView);
    QObject::connect(mSourceModel.data(), &QAbstractItemModel::rowsMoved, this, &EventOccurrenceModel::refreshView);
    QObject::connect(mSourceModel.data(), &QAbstractItemModel::rowsRemoved, this, &EventOccurrenceModel::refreshView);

    refreshView();
}

void EventOccurrenceModel::refreshView()
{
    mUpdateFromSourceDebouncer.trigger();
}

void EventOccurrenceModel::updateFromSource()
{
    QList<Occurrence> newEvents;

    if (mSourceModel) {
        QMap<QByteArray, KCalendarCore::Incidence::Ptr> recurringEvents;
        QMultiMap<QByteArray, KCalendarCore::Incidence::Ptr> exceptions;
        QMap<QByteArray, QSharedPointer<Sink::ApplicationDomain::Event>> events;
        for (int i = 0; i < mSourceModel->rowCount(); ++i) {
            auto event = mSourceModel->index(i, 0).data(Sink::Store::DomainObjectRole).value<ApplicationDomain::Event::Ptr>();
            Q_ASSERT(event);

            //Parse the event
            auto icalEvent = KCalendarCore::ICalFormat().readIncidence(event->getIcal()).dynamicCast<KCalendarCore::Event>();
            if(!icalEvent) {
                SinkWarning() << "Invalid ICal to process, ignoring...";
                continue;
            }

            //Collect recurring events and add the rest immediately
            if (icalEvent->recurs()) {
                recurringEvents.insert(icalEvent->uid().toLatin1(), icalEvent);
                events.insert(icalEvent->instanceIdentifier().toLatin1(), event);
            } else if(icalEvent->recurrenceId().isValid()) {
                exceptions.insert(icalEvent->uid().toLatin1(), icalEvent);
                events.insert(icalEvent->instanceIdentifier().toLatin1(), event);
            } else {
                if (icalEvent->dtStart().date() < mEnd && icalEvent->dtEnd().date() >= mStart) {
                    newEvents.append({icalEvent->dtStart(), icalEvent->dtEnd(), icalEvent, getColor(event->getCalendar()), event->getAllDay(), event});
                }
            }
        }
        //process all recurring events and their exceptions.
        for (const auto &uid : recurringEvents.keys()) {
            KCalendarCore::MemoryCalendar calendar{QTimeZone::systemTimeZone()};
            calendar.addIncidence(recurringEvents.value(uid));
            for (const auto &event : exceptions.values(uid)) {
                calendar.addIncidence(event);
            }
            KCalendarCore::OccurrenceIterator occurrenceIterator{calendar, QDateTime{mStart, {0, 0, 0}}, QDateTime{mEnd, {12, 59, 59}}};
            while (occurrenceIterator.hasNext()) {
                occurrenceIterator.next();
                const auto incidence = occurrenceIterator.incidence();
                Q_ASSERT(incidence);
                const auto event = events.value(incidence->instanceIdentifier().toLatin1());
                const auto start = occurrenceIterator.occurrenceStartDate();
                const auto end = incidence->endDateForStart(start);
                if (start.date() < mEnd && end.date() >= mStart) {
                    newEvents.append({start, end, incidence, getColor(event->getCalendar()), event->getAllDay(), event});
                }
            }
        }
        //Process all exceptions that had no main event present in the current query
        for (const auto &uid : exceptions.keys()) {
            const auto icalEvent = exceptions.value(uid).dynamicCast<KCalendarCore::Event>();
            Q_ASSERT(icalEvent);
            const auto event = events.value(icalEvent->instanceIdentifier().toLatin1());
            if (icalEvent->dtStart().date() < mEnd && icalEvent->dtEnd().date() >= mStart) {
                newEvents.append({icalEvent->dtStart(), icalEvent->dtEnd(), icalEvent, getColor(event->getCalendar()), event->getAllDay(), event});
            }
        }
    }

    {
        auto it = std::begin(mEvents);
        while (it != std::end(mEvents)) {
            const auto event = *it;
            auto itToRemove = std::find_if(std::begin(newEvents), std::end(newEvents), [&] (const auto &e) {
                Q_ASSERT(e.incidence);
                Q_ASSERT(event.incidence);
                return e.incidence->uid() == event.incidence->uid() && e.start == event.start;
            });
            // Can't find the vevent in newEvents anymore, so remove from list
            if (itToRemove == std::end(newEvents)) {
                //Removed
                const int startIndex = std::distance(std::begin(mEvents), it);
                beginRemoveRows(QModelIndex(), startIndex, startIndex);
                it = mEvents.erase(it);
                endRemoveRows();
            } else {
                it++;
            }
        }
    }

    for (auto newIt = std::begin(newEvents); newIt != std::end(newEvents); newIt++) {
        const auto event = *newIt;
        auto it = std::find_if(std::begin(mEvents), std::end(mEvents), [&] (const auto &e) {
            Q_ASSERT(e.incidence);
            return e.incidence->uid() == event.incidence->uid() && e.start == event.start;
        });
        if (it == std::end(mEvents)) {
            //New event
            const int startIndex = std::distance(std::begin(newEvents), newIt);
            beginInsertRows(QModelIndex(), startIndex, startIndex);
            mEvents.insert(startIndex, event);
            endInsertRows();
        } else {
            if (*(newIt->incidence) != *(it->incidence)) {
                const int startIndex = std::distance(std::begin(mEvents), it);
                mEvents[startIndex] = event;
                emit dataChanged(index(startIndex, 0), index(startIndex, 0), {});
            }
        }
    }

    if (!mInitialItemsLoaded && (!mSourceModel || mSourceModel->data({}, Sink::Store::ChildrenFetchedRole).toBool())) {
        mInitialItemsLoaded = true;
        emit initialItemsLoaded();
    }
}


bool EventOccurrenceModel::initialItemsComplete() const
{
    return mInitialItemsLoaded;
}

QModelIndex EventOccurrenceModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return {};
    }

    if (!parent.isValid()) {
        return createIndex(row, column);
    }
    return {};
}

QModelIndex EventOccurrenceModel::parent(const QModelIndex &) const
{
    return {};
}

int EventOccurrenceModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return mEvents.size();
    }
    return 0;
}

int EventOccurrenceModel::columnCount(const QModelIndex &) const
{
    return 1;
}

QByteArray EventOccurrenceModel::getColor(const QByteArray &calendar) const
{
    const auto color = mCalendarCache->getProperty(calendar, "color").toByteArray();
    if (color.isEmpty()) {
        qWarning() << "Failed to get color for calendar " << calendar;
    }
    return color;
}

QVariant EventOccurrenceModel::data(const QModelIndex &idx, int role) const
{
    if (!hasIndex(idx.row(), idx.column())) {
        return {};
    }
    auto event = mEvents.at(idx.row());
    auto icalEvent = event.incidence;
    switch (role) {
        case Summary:
            return icalEvent->summary();
        case Description:
            return icalEvent->description();
        case StartTime:
            return event.start;
        case EndTime:
            return event.end;
        case Color:
            return event.color;
        case AllDay:
            return event.allDay;
        case Event:
            return QVariant::fromValue(event.domainObject);
        case EventOccurrence:
            return QVariant::fromValue(event);
        default:
            SinkWarning() << "Unknown role for event:" << QMetaEnum::fromType<Roles>().valueToKey(role);
            return {};
    }
}

