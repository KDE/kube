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

#include "perioddayeventmodel.h"

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

PeriodDayEventModel::PeriodDayEventModel(QObject *parent)
    : QAbstractItemModel(parent),
    partitionedEvents(7),
    mCalendarCache{EntityCache<Calendar, Calendar::Color>::Ptr::create()},
    mCalendar{new KCalCore::MemoryCalendar{QTimeZone::systemTimeZone()}}
{
    mRefreshTimer.setSingleShot(true);
    QObject::connect(&mRefreshTimer, &QTimer::timeout, this, &PeriodDayEventModel::partitionData);

    updateQuery();
}

void PeriodDayEventModel::updateQuery()
{
    qWarning() << "Update query";
    Sink::Query query;
    query.setFlags(Sink::Query::LiveQuery);
    query.request<Event::Summary>();
    query.request<Event::Description>();
    query.request<Event::StartTime>();
    query.request<Event::EndTime>();
    query.request<Event::Calendar>();
    query.request<Event::Ical>();
    query.request<Event::AllDay>();

    auto periodEnd = mPeriodStart.addDays(mPeriodLength);

    query.filter<Event::StartTime, Event::EndTime>(
        Sink::Query::Comparator(QVariantList{mPeriodStart, periodEnd}, Sink::Query::Comparator::Overlap));

    eventModel = Sink::Store::loadModel<Event>(query);

    QObject::connect(eventModel.data(), &QAbstractItemModel::dataChanged, this, &PeriodDayEventModel::refreshView);
    QObject::connect(eventModel.data(), &QAbstractItemModel::layoutChanged, this, &PeriodDayEventModel::refreshView);
    QObject::connect(eventModel.data(), &QAbstractItemModel::modelReset, this, &PeriodDayEventModel::refreshView);
    QObject::connect(eventModel.data(), &QAbstractItemModel::rowsInserted, this, &PeriodDayEventModel::refreshView);
    QObject::connect(eventModel.data(), &QAbstractItemModel::rowsMoved, this, &PeriodDayEventModel::refreshView);
    QObject::connect(eventModel.data(), &QAbstractItemModel::rowsRemoved, this, &PeriodDayEventModel::refreshView);

    refreshView();
}

void PeriodDayEventModel::refreshView()
{
    if (!mRefreshTimer.isActive()) {
        //Instant update, but then only refresh every 50ms max.
        partitionData();
        mRefreshTimer.start(50);
    }
}

void PeriodDayEventModel::partitionData()
{
    beginResetModel();

    partitionedEvents = QVector<QList<QSharedPointer<Event>>>(mPeriodLength);
    mAllDayEvents = QList<QSharedPointer<Event>>();

    for (int i = 0; i < eventModel->rowCount(); ++i) {
        auto event = eventModel->index(i, 0).data(Sink::Store::DomainObjectRole).value<Event::Ptr>();
        if (!mCalendarFilter.contains(event->getCalendar())) {
            continue;
        }

        //Parse the event
        auto icalEvent = KCalCore::ICalFormat().readIncidence(event->getIcal()).dynamicCast<KCalCore::Event>();
        if(!icalEvent) {
            SinkWarning() << "Invalid ICal to process, ignoring...";
            continue;
        }

        if (event->getAllDay() || icalEvent->allDay()) {
            if (icalEvent->recurs()) {
                auto startOfPeriod = icalEvent->dtStart();
                startOfPeriod.setDate(mPeriodStart);
                const auto endOfPeriod = startOfPeriod.addDays(mPeriodLength);
                const auto duration = icalEvent->hasDuration() ? icalEvent->duration().asSeconds() : 0;

                KCalCore::OccurrenceIterator occurrenceIterator{*mCalendar, icalEvent, startOfPeriod, endOfPeriod};
                while (occurrenceIterator.hasNext()) {
                    occurrenceIterator.next();
                    const auto start = occurrenceIterator.occurrenceStartDate();
                    const auto end = start.addSecs(duration);
                    //FIXME don't abuse ApplicationDomainType here
                    Event::Ptr occurrence = Sink::ApplicationDomain::ApplicationDomainType::getInMemoryRepresentation<Event>(*event);
                    occurrence->setExtractedStartTime(start);
                    occurrence->setExtractedEndTime(end);
                    if (start.date() < mPeriodStart.addDays(mPeriodLength) && end.date() >= mPeriodStart) {
                        mAllDayEvents.append(occurrence);
                    }
                }
            } else {
                if (icalEvent->dtStart().date() < mPeriodStart.addDays(mPeriodLength) && icalEvent->dtEnd().date() >= mPeriodStart) {
                    mAllDayEvents.append(event);
                }
            }
            continue;
        }

        auto addEvent = [&] (Event::Ptr event) {
            const QDate eventDate = event->getStartTime().date();
            const int bucket = bucketOf(eventDate);
            SinkTrace() << "Adding event:" << event->getSummary() << eventDate << "in bucket #" << bucket;
            //Only let events part of this view pass
            if (bucket < 0 || bucket >= partitionedEvents.size()) {
                return;
            }
            partitionedEvents[bucket].append(event);

            //Also add the event to all other days it spans
            const QDate endDate = event->getEndTime().date();
            if (endDate.isValid()) {
                const int endBucket = qMin(bucketOf(endDate), periodLength() - 1);
                for (int i = bucket + 1; i <= endBucket; i++) {
                    Q_ASSERT(i >= 0 && i < partitionedEvents.size());
                    partitionedEvents[i].append(event);
                }

            }
        };


        if (icalEvent->recurs()) {
            auto startOfPeriod = icalEvent->dtStart();
            startOfPeriod.setDate(mPeriodStart);
            const auto endOfPeriod = startOfPeriod.addDays(mPeriodLength);

            const auto duration = icalEvent->hasDuration() ? icalEvent->duration().asSeconds() : 0;

            KCalCore::OccurrenceIterator occurrenceIterator{*mCalendar, icalEvent, startOfPeriod, endOfPeriod};
            while (occurrenceIterator.hasNext()) {
                occurrenceIterator.next();
                const auto start = occurrenceIterator.occurrenceStartDate();
                //FIXME don't abuse ApplicationDomainType here
                Event::Ptr occurrence = Sink::ApplicationDomain::ApplicationDomainType::getInMemoryRepresentation<Event>(*event);
                occurrence->setExtractedStartTime(start);
                occurrence->setExtractedEndTime(start.addSecs(duration));

                addEvent(occurrence);
            }
        } else {
            addEvent(event);
        }

    }

    endResetModel();
    emit daylongEventsChanged();
}

int PeriodDayEventModel::bucketOf(const QDate &candidate) const
{
    return mPeriodStart.daysTo(candidate);
}

QModelIndex PeriodDayEventModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return {};
    }

    if (!parent.isValid()) {
        // Asking for a day

        if (!(0 <= row && row < mPeriodLength)) {
            return {};
        }

        return createIndex(row, column, DAY_ID);
    }

    // Asking for an Event
    auto day = static_cast<int>(parent.row());

    Q_ASSERT(0 <= day && day < partitionedEvents.size());
    if (row >= partitionedEvents[day].size()) {
        return {};
    }

    return createIndex(row, column, day);
}

QModelIndex PeriodDayEventModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return {};
    }

    if (index.internalId() == DAY_ID) {
        return {};
    }

    auto day = index.internalId();

    return this->index(day, 0);
}

int PeriodDayEventModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return mPeriodLength;
    }

    auto day = parent.row();
    Q_ASSERT(0 <= day && day < partitionedEvents.size());
    return partitionedEvents[day].size();
}

int PeriodDayEventModel::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return 1;
    }

    return eventModel->columnCount();
}

QByteArray PeriodDayEventModel::getColor(const QByteArray &calendar) const
{
    const auto color = mCalendarCache->getProperty(calendar, "color").toByteArray();
    if (color.isEmpty()) {
        qWarning() << "Failed to get color for calendar " << calendar;
    }
    return color;
}

QDateTime PeriodDayEventModel::getStartTimeOfDay(const QDateTime &dateTime, int day) const
{
    if (bucketOf(dateTime.date()) < day) {
        return QDateTime{mPeriodStart.addDays(day), QTime{0,0}};
    }
    return dateTime;
}

QDateTime PeriodDayEventModel::getEndTimeOfDay(const QDateTime &dateTime, int day) const
{
    if (bucketOf(dateTime.date()) > day) {
        return QDateTime{mPeriodStart.addDays(day), QTime{23, 59, 59}};
    }
    return dateTime;
}

QVariant PeriodDayEventModel::data(const QModelIndex &id, int role) const
{
    if (id.internalId() == DAY_ID) {
        auto day = id.row();

        SinkTrace() << "Fetching data for day" << day << "with role"
                    << QMetaEnum::fromType<Roles>().valueToKey(role);

        switch (role) {
            case Qt::DisplayRole:
                return mPeriodStart.addDays(day).toString();
            case Date:
                return mPeriodStart.addDays(day);
            case Events: {
                auto result = QVariantList{};

                QMultiMap<QTime, int> sorted;
                Q_ASSERT(0 <= day && day <= partitionedEvents.size());
                for (int i = 0; i < partitionedEvents[day].size(); ++i) {
                    const auto eventId = index(i, 0, id);
                    sorted.insert(data(eventId, StartTime).toDateTime().time(), i);
                }

                QMap<QTime, int> indentationStack;
                for (auto it = sorted.begin(); it != sorted.end(); it++) {
                    auto eventId = index(it.value(), 0, id);
                    SinkTrace() << "Appending event:" << data(eventId, Summary);

                    const auto startTime = data(eventId, StartTime).toDateTime().time();
                    auto endTime = data(eventId, EndTime).toDateTime().time();
                    if (!endTime.isValid()) {
                        //Even without duration we still take some space visually
                        endTime = startTime.addSecs(60 * 20);
                    }

                    //Remove all dates before startTime
                    for (auto it = indentationStack.begin(); it != indentationStack.end();) {
                        if (it.key() < startTime) {
                            it = indentationStack.erase(it);
                        } else {
                            ++it;
                        }
                    }
                    const int indentation = indentationStack.size();
                    indentationStack.insert(endTime, 0);

                    result.append(QVariantMap{
                        {"text", data(eventId, Summary)},
                        {"description", data(eventId, Description)},
                        {"starts", startTime.hour() + startTime.minute() / 60.},
                        {"duration", data(eventId, Duration)},
                        {"color", data(eventId, Color)},
                        {"indentation", indentation},
                    });
                }

                return result;
            }
            default:
                SinkWarning() << "Unknown role for day:" << QMetaEnum::fromType<Roles>().valueToKey(role);
                return {};
        }
    } else {
        auto day = static_cast<int>(id.internalId());
        SinkTrace() << "Fetching data for event on day" << day << "with role"
                    << QMetaEnum::fromType<Roles>().valueToKey(role);
        Q_ASSERT(0 <= day && day < partitionedEvents.size());
        auto event = partitionedEvents[day].at(id.row());

        switch (role) {
            case Summary:
                return event->getSummary();
            case Description:
                return event->getDescription();
            case StartTime:
                return getStartTimeOfDay(event->getStartTime(), day);
            case EndTime:
                return getEndTimeOfDay(event->getEndTime(), day);
            case Duration: {
                auto start = getStartTimeOfDay(event->getStartTime(), day);
                auto end = getEndTimeOfDay(event->getEndTime(), day);
                return qRound(start.secsTo(end) / 3600.0);
            }
            case Color:
                return getColor(event->getCalendar());
            default:
                SinkWarning() << "Unknown role for event:" << QMetaEnum::fromType<Roles>().valueToKey(role);
                return {};
        }
    }
}

QHash<int, QByteArray> PeriodDayEventModel::roleNames() const
{
    return {
        {Events, "events"},
        {Date, "date"},
        {Summary, "summary"},
        {Description, "description"},
        {StartTime, "starts"},
        {Duration, "duration"},
        {Color, "color"}
    };
}

QDate PeriodDayEventModel::periodStart() const
{
    return mPeriodStart;
}

void PeriodDayEventModel::setPeriodStart(const QDate &start)
{
    if (!start.isValid()) {
        SinkWarning() << "Passed an invalid starting date in setPeriodStart, ignoring...";
        return;
    }

    mPeriodStart = start;
    updateQuery();
}

void PeriodDayEventModel::setPeriodStart(const QVariant &start)
{
    setPeriodStart(start.toDate());
}

int PeriodDayEventModel::periodLength() const
{
    return mPeriodLength;
}

void PeriodDayEventModel::setPeriodLength(int length)
{
    mPeriodLength = length;
    updateQuery();
}

QSet<QByteArray> PeriodDayEventModel::calendarFilter() const
{
    return mCalendarFilter;
}

void PeriodDayEventModel::setCalendarFilter(const QSet<QByteArray> &filter)
{
    mCalendarFilter = filter;
    updateQuery();
}

/*
 * We return a list of lists,
 * where the first list is the number of lines,
 * and the contained list the events of that line.
 *
 * We try to merge multiple events into single lines if they don't overlap.
 */
QVariantList PeriodDayEventModel::daylongEvents()
{
    auto getStart = [this] (const Event &event) {
        return qMax(mPeriodStart.daysTo(event.getStartTime().date()), 0ll);
    };

    auto getDuration = [] (const Event &event) {
        return qMax(event.getStartTime().date().daysTo(event.getEndTime().date()), 1ll);
    };

    QMultiMap<int, Event::Ptr> sorted;
    //Sort by duration
    for (const auto &event : mAllDayEvents) {
        sorted.insert(getDuration(*event), event);
    }

    auto result = QVariantList{};
    auto currentLine = QVariantList{};
    int lastStart = -1;
    int lastDuration = 0;
    for (const auto &event : sorted) {
        const auto start = getStart(*event);
        const auto duration = qMin(getDuration(*event), mPeriodLength - start);
        const auto end = start + duration;
        currentLine.append(QVariantMap{
            {"text", event->getSummary()},
            {"description", event->getDescription()},
            {"starts", start},
            {"duration", duration},
            {"color", getColor(event->getCalendar())},
        });

        if (lastStart >= 0) {
            const auto lastEnd = lastStart + lastDuration;

            //Does intersect
            if (((start >= lastStart) && (start <= lastEnd)) ||
                ((end >= lastStart) && (end <= lastStart)) ||
                ((start <= lastStart) && (end >= lastEnd))) {
                result.append(QVariant::fromValue(currentLine));
                currentLine = {};
            }
        }
        lastStart = start;
        lastDuration = duration;
    }
    if (!currentLine.isEmpty()) {
        result.append(QVariant::fromValue(currentLine));
    }

    return result;
}
