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

#include "eventmodel.h"

#include <sink/log.h>
#include <sink/query.h>
#include <sink/store.h>

#include <QJsonArray>
#include <QJsonObject>
#include <QMetaEnum>

#include <KCalCore/ICalFormat>
#include <KCalCore/OccurrenceIterator>
#include <KCalCore/MemoryCalendar>

#include <entitycache.h>

using Event = Sink::ApplicationDomain::Event;
using Calendar = Sink::ApplicationDomain::Calendar;


EventModel::EventModel(QObject *parent)
    : QAbstractItemModel(parent),
    mCalendarCache{EntityCache<Calendar, Calendar::Color>::Ptr::create()},
    mCalendar{new KCalCore::MemoryCalendar{QTimeZone::systemTimeZone()}}
{
    mRefreshTimer.setSingleShot(true);
    QObject::connect(&mRefreshTimer, &QTimer::timeout, this, &EventModel::updateFromSource);
}

void EventModel::updateQuery(const QDate &start, const QDate &end, const QSet<QByteArray> &calendarFilter)
{
    qWarning() << "Update query";
    mCalendarFilter = calendarFilter;
    Sink::Query query;
    query.setFlags(Sink::Query::LiveQuery);
    query.request<Event::Summary>();
    query.request<Event::Description>();
    query.request<Event::StartTime>();
    query.request<Event::EndTime>();
    query.request<Event::Calendar>();
    query.request<Event::Ical>();
    query.request<Event::AllDay>();

    query.filter<Event::StartTime, Event::EndTime>(Sink::Query::Comparator(QVariantList{start, end}, Sink::Query::Comparator::Overlap));

    mSourceModel = Sink::Store::loadModel<Event>(query);

    QObject::connect(mSourceModel.data(), &QAbstractItemModel::dataChanged, this, &EventModel::refreshView);
    QObject::connect(mSourceModel.data(), &QAbstractItemModel::layoutChanged, this, &EventModel::refreshView);
    QObject::connect(mSourceModel.data(), &QAbstractItemModel::modelReset, this, &EventModel::refreshView);
    QObject::connect(mSourceModel.data(), &QAbstractItemModel::rowsInserted, this, &EventModel::refreshView);
    QObject::connect(mSourceModel.data(), &QAbstractItemModel::rowsMoved, this, &EventModel::refreshView);
    QObject::connect(mSourceModel.data(), &QAbstractItemModel::rowsRemoved, this, &EventModel::refreshView);

    refreshView();
}

void EventModel::refreshView()
{
    if (!mRefreshTimer.isActive()) {
        //Instant update, but then only refresh every 50ms max.
        updateFromSource();
        mRefreshTimer.start(50);
    }
}

void EventModel::updateFromSource()
{
    beginResetModel();

    mEvents.clear();

    for (int i = 0; i < mSourceModel->rowCount(); ++i) {
        auto event = mSourceModel->index(i, 0).data(Sink::Store::DomainObjectRole).value<Event::Ptr>();
        if (!mCalendarFilter.contains(event->getCalendar())) {
            continue;
        }

        //Parse the event
        auto icalEvent = KCalCore::ICalFormat().readIncidence(event->getIcal()).dynamicCast<KCalCore::Event>();
        if(!icalEvent) {
            SinkWarning() << "Invalid ICal to process, ignoring...";
            continue;
        }

        if (icalEvent->recurs()) {
            const auto duration = icalEvent->hasDuration() ? icalEvent->duration().asSeconds() : 0;
            KCalCore::OccurrenceIterator occurrenceIterator{*mCalendar, icalEvent, QDateTime{mStart, {0, 0, 0}}, QDateTime{mEnd, {12, 59, 59}}};
            while (occurrenceIterator.hasNext()) {
                occurrenceIterator.next();
                const auto start = occurrenceIterator.occurrenceStartDate();
                const auto end = start.addSecs(duration);
                if (start.date() < mEnd && end.date() >= mStart) {
                    mEvents.append({start, end, occurrenceIterator.incidence(), getColor(event->getCalendar()), event->getAllDay()});
                }
            }
        } else {
            if (icalEvent->dtStart().date() < mEnd && icalEvent->dtEnd().date() >= mStart) {
                mEvents.append({icalEvent->dtStart(), icalEvent->dtEnd(), icalEvent, getColor(event->getCalendar()), event->getAllDay()});
            }
        }
    }

    endResetModel();
}

QModelIndex EventModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return {};
    }

    if (!parent.isValid()) {
        return createIndex(row, column);
    }
    return {};
}

QModelIndex EventModel::parent(const QModelIndex &index) const
{
    return {};
}

int EventModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return mEvents.size();
    }
    return 0;
}

int EventModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QByteArray EventModel::getColor(const QByteArray &calendar) const
{
    const auto color = mCalendarCache->getProperty(calendar, "color").toByteArray();
    if (color.isEmpty()) {
        qWarning() << "Failed to get color for calendar " << calendar;
    }
    return color;
}

// QDateTime EventModel::getStartTimeOfDay(const QDateTime &dateTime, int day) const
// {
//     if (bucketOf(dateTime.date()) < day) {
//         return QDateTime{mPeriodStart.addDays(day), QTime{0,0}};
//     }
//     return dateTime;
// }

// QDateTime EventModel::getEndTimeOfDay(const QDateTime &dateTime, int day) const
// {
//     if (bucketOf(dateTime.date()) > day) {
//         return QDateTime{mPeriodStart.addDays(day), QTime{23, 59, 59}};
//     }
//     return dateTime;
// }

QVariant EventModel::data(const QModelIndex &idx, int role) const
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
        default:
            SinkWarning() << "Unknown role for event:" << QMetaEnum::fromType<Roles>().valueToKey(role);
            return {};
    }
}

