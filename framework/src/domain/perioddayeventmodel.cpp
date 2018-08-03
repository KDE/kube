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

#include <entitycache.h>

PeriodDayEventModel::PeriodDayEventModel(QObject *parent)
    : QAbstractItemModel(parent), partitionedEvents(7)
{
    updateQuery();
}

void PeriodDayEventModel::updateQuery()
{
    Sink::Query query;
    query.setFlags(Sink::Query::LiveQuery);
    query.request<Event::Summary>();
    query.request<Event::Description>();
    query.request<Event::StartTime>();
    query.request<Event::EndTime>();
    query.request<Event::Calendar>();

    auto periodEnd = mPeriodStart.addDays(mPeriodLength);

    query.filter<Event::StartTime, Event::EndTime>(
        Sink::Query::Comparator(QVariantList{mPeriodStart, periodEnd}, Sink::Query::Comparator::Overlap));
    query.filter<Event::AllDay>(false);

    eventModel = Sink::Store::loadModel<Event>(query);

    QObject::connect(eventModel.data(), &QAbstractItemModel::dataChanged, this, &PeriodDayEventModel::partitionData);
    QObject::connect(eventModel.data(), &QAbstractItemModel::layoutChanged, this, &PeriodDayEventModel::partitionData);
    QObject::connect(eventModel.data(), &QAbstractItemModel::modelReset, this, &PeriodDayEventModel::partitionData);
    QObject::connect(eventModel.data(), &QAbstractItemModel::rowsInserted, this, &PeriodDayEventModel::partitionData);
    QObject::connect(eventModel.data(), &QAbstractItemModel::rowsMoved, this, &PeriodDayEventModel::partitionData);
    QObject::connect(eventModel.data(), &QAbstractItemModel::rowsRemoved, this, &PeriodDayEventModel::partitionData);

    mCalendarCache = EntityCache<Calendar, Calendar::Color>::Ptr::create();

    partitionData();
}

void PeriodDayEventModel::partitionData()
{
    SinkLog() << "Partitioning event data";

    beginResetModel();

    partitionedEvents = QVector<QList<QSharedPointer<Event>>>(mPeriodLength);

    for (int i = 0; i < eventModel->rowCount(); ++i) {
        auto event = eventModel->index(i, 0).data(Sink::Store::DomainObjectRole).value<Event::Ptr>();
        QDate eventDate = event->getStartTime().date();

        if (!eventDate.isValid()) {
            SinkWarning() << "Invalid date in the eventModel, ignoring...";
            continue;
        }

        int bucket = bucketOf(eventDate);

        SinkTrace() << "Adding event:" << event->getSummary() << "in bucket #" << bucket;
        partitionedEvents[bucket].append(event);
    }

    endResetModel();
}

int PeriodDayEventModel::bucketOf(const QDate &candidate) const
{
    int bucket = mPeriodStart.daysTo(candidate);

    return bucket;
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

    Q_ASSERT(0 <= day && day <= mPeriodLength);
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
    return mCalendarCache->getProperty(calendar, "color").toByteArray();
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

                for (int i = 0; i < partitionedEvents[day].size(); ++i) {
                    auto eventId = index(i, 0, id);
                    SinkTrace() << "Appending event:" << data(eventId, Summary);

                    auto startTime = data(eventId, StartTime).toDateTime().time();

                    result.append(QVariantMap{
                        {"text", data(eventId, Summary)},
                        {"description", data(eventId, Description)},
                        {"starts", startTime.hour() + startTime.minute() / 60.},
                        {"duration", data(eventId, Duration)},
                        {"color", data(eventId, Color)},
                        {"indention", 0},
                    });
                }

                return result;
            }
            default:
                SinkWarning() << "Unknown role for day:" << QMetaEnum::fromType<Roles>().valueToKey(role);
                return {};
        }
    } else {
        auto day = id.internalId();
        SinkTrace() << "Fetching data for event on day" << day << "with role"
                    << QMetaEnum::fromType<Roles>().valueToKey(role);
        auto event = partitionedEvents[day].at(id.row());

        switch (role) {
            case Summary:
                return event->getSummary();
            case Description:
                return event->getDescription();
            case StartTime:
                return event->getStartTime();
            case Duration: {
                auto start = event->getStartTime();
                auto end   = event->getEndTime();
                return start.secsTo(end) / 3600;
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
