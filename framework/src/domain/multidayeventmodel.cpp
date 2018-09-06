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

#include "multidayeventmodel.h"

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
#include <eventmodel.h>

enum Roles {
    Events = EventModel::LastRole
};

MultiDayEventModel::MultiDayEventModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    // mRefreshTimer.setSingleShot(true);
    // QObject::connect(&mRefreshTimer, &QTimer::timeout, this, &MultiDayEventModel::updateFromSource);
}

// void MultiDayEventModel::setSourceModel(QAbstractItemModel *model)
// {
    // qWarning() << "Update query";
    // mCalendarFilter = calendarFilter;
    // Sink::Query query;
    // query.setFlags(Sink::Query::LiveQuery);
    // query.request<Event::Summary>();
    // query.request<Event::Description>();
    // query.request<Event::StartTime>();
    // query.request<Event::EndTime>();
    // query.request<Event::Calendar>();
    // query.request<Event::Ical>();
    // query.request<Event::AllDay>();

    // query.filter<Event::StartTime, Event::EndTime>(Sink::Query::Comparator(QVariantList{start, end}, Sink::Query::Comparator::Overlap));

    // mSourceModel = Sink::Store::loadModel<Event>(query);

    // QObject::connect(model, &QAbstractItemModel::dataChanged, this, &MultiDayEventModel::modelReset);
    // QObject::connect(model, &QAbstractItemModel::layoutChanged, this, &MultiDayEventModel::refreshView);
    // QObject::connect(model, &QAbstractItemModel::modelReset, this, &MultiDayEventModel::refreshView);
    // QObject::connect(model, &QAbstractItemModel::rowsInserted, this, &MultiDayEventModel::refreshView);
    // QObject::connect(model, &QAbstractItemModel::rowsMoved, this, &MultiDayEventModel::refreshView);
    // QObject::connect(model, &QAbstractItemModel::rowsRemoved, this, &MultiDayEventModel::refreshView);

    // refreshView();
// }

//void MultiDayEventModel::refreshView()
//{
//    if (!mRefreshTimer.isActive()) {
//        //Instant update, but then only refresh every 50ms max.
//        updateFromSource();
//        mRefreshTimer.start(50);
//    }
//}

//void MultiDayEventModel::updateFromSource()
//{
//    beginResetModel();

//    mEvents.clear();

//    for (int i = 0; i < mSourceModel->rowCount(); ++i) {
//        auto event = mSourceModel->index(i, 0).data(Sink::Store::DomainObjectRole).value<Event::Ptr>();
//        if (!mCalendarFilter.contains(event->getCalendar())) {
//            continue;
//        }

//        //Parse the event
//        auto icalEvent = KCalCore::ICalFormat().readIncidence(event->getIcal()).dynamicCast<KCalCore::Event>();
//        if(!icalEvent) {
//            SinkWarning() << "Invalid ICal to process, ignoring...";
//            continue;
//        }

//        if (icalEvent->recurs()) {
//            const auto duration = icalEvent->hasDuration() ? icalEvent->duration().asSeconds() : 0;
//            KCalCore::OccurrenceIterator occurrenceIterator{*mCalendar, icalEvent, QDateTime{mStart, {0, 0, 0}}, QDateTime{mEnd, {12, 59, 59}}};
//            while (occurrenceIterator.hasNext()) {
//                occurrenceIterator.next();
//                const auto start = occurrenceIterator.occurrenceStartDate();
//                const auto end = start.addSecs(duration);
//                if (start.date() < mEnd && end.date() >= mStart) {
//                    mEvents.append({start, end, occurrenceIterator.incidence(), getColor(event->getCalendar()), event->getAllDay()});
//                }
//            }
//        } else {
//            if (icalEvent->dtStart().date() < mEnd && icalEvent->dtEnd().date() >= mStart) {
//                mEvents.append({icalEvent->dtStart(), icalEvent->dtEnd(), icalEvent, getColor(event->getCalendar()), event->getAllDay()});
//            }
//        }
//    }

//    endResetModel();
//}

QModelIndex MultiDayEventModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return {};
    }

    if (!parent.isValid()) {
        return createIndex(row, column);
    }
    return {};
}

QModelIndex MultiDayEventModel::parent(const QModelIndex &index) const
{
    return {};
}

int MultiDayEventModel::rowCount(const QModelIndex &parent) const
{
    //Numbe of weeks
    if (!parent.isValid()) {
        return 1;
    }
    return 0;
}

int MultiDayEventModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant MultiDayEventModel::data(const QModelIndex &idx, int role) const
{
    if (!hasIndex(idx.row(), idx.column())) {
        return {};
    }
    switch (role) {
        case Events: {
            if (!mSourceModel) {
                qWarning() << "no source model";
                return {};
            }
            qWarning() << "Getting events";
            //FIXME set start date of this row
            QDate rowStart = mStart;
            auto getStart = [&] (const QDate &start) {
                return qMax(rowStart.daysTo(start), 0ll);
            };

            auto getDuration = [&] (const QDate &start, const QDate &end) {
                return qMax(start.daysTo(end), 1ll);
            };

            //TODO iterate over source model, filter by allday, and get sorted list of indexes
            QMultiMap<int, QModelIndex> sorted;
            //Sort by duration
            for (int row = 0; row < mSourceModel->rowCount(); row++) {
                const auto srcIdx = mSourceModel->index(row, 0, {});
                //Filter for allday only
                if (!srcIdx.data(EventModel::AllDay).toBool()) {
                    continue;
                }
                qWarning() << "Get start "<< srcIdx.data(EventModel::Summary) << srcIdx.data(EventModel::StartTime).toDateTime();
                sorted.insert(getDuration(srcIdx.data(EventModel::StartTime).toDateTime().date(), srcIdx.data(EventModel::EndTime).toDateTime().date()), srcIdx);
            }

            auto result = QVariantList{};
            auto currentLine = QVariantList{};
            int lastStart = -1;
            int lastDuration = 0;
            for (const auto &srcIdx : sorted) {
                const auto start = getStart(srcIdx.data(EventModel::StartTime).toDateTime().date());
                const auto duration = qMin(getDuration(srcIdx.data(EventModel::StartTime).toDateTime().date(), srcIdx.data(EventModel::EndTime).toDateTime().date()), mPeriodLength - start);
                qWarning() << "Get start2 " << srcIdx.data(EventModel::Summary) << start << duration;
                const auto end = start + duration;
                currentLine.append(QVariantMap{
                    {"text", srcIdx.data(EventModel::Summary)},
                    {"description", srcIdx.data(EventModel::Description)},
                    {"starts", start},
                    {"duration", duration},
                    {"color", srcIdx.data(EventModel::Color)},
                });

                if (lastStart >= 0) {
                    const auto lastEnd = lastStart + lastDuration;

                    //Does intersect
                    if (((start >= lastStart) && (start <= lastEnd)) ||
                        ((end >= lastStart) && (end <= lastStart)) ||
                        ((start <= lastStart) && (end >= lastEnd))) {
                        result.append(QVariant::fromValue(currentLine));
                        qWarning() << "Found intersection " << currentLine;
                        currentLine = {};
                    }
                }
                lastStart = start;
                lastDuration = duration;
            }
            if (!currentLine.isEmpty()) {
                result.append(QVariant::fromValue(currentLine));
            }

            qWarning() << "Getting events" << result;
            return result;
        }
        default:
            // SinkWarning() << "Unknown role for event:" << QMetaEnum::fromType<Roles>().valueToKey(role);
            Q_ASSERT(false);
            return {};
    }
}

void MultiDayEventModel::setConfiguration(const QVariantMap &configuration)
{
    mStart = configuration.value("start").toDate();
    mLength = configuration.value("length").toInt();
    setupModel(mStart, mLength, mFilter);
}

void MultiDayEventModel::setupModel(const QDate &start, int length, const QSet<QByteArray> &filter)
{
    beginResetModel();
    auto sourceModel = QSharedPointer<EventModel>::create();

    sourceModel->updateQuery(start, start.addDays(length), filter);

    auto resetModel = [this] {
        beginResetModel();
        endResetModel();
    };
    auto model = sourceModel.data();
    QObject::connect(model, &QAbstractItemModel::dataChanged, this, resetModel);
    QObject::connect(model, &QAbstractItemModel::layoutChanged, this, resetModel);
    QObject::connect(model, &QAbstractItemModel::modelReset, this, resetModel);
    QObject::connect(model, &QAbstractItemModel::rowsInserted, this, resetModel);
    // QObject::connect(model, &QAbstractItemModel::rowsMoved, this, &MultiDayEventModel::refreshView);
    // QObject::connect(model, &QAbstractItemModel::rowsRemoved, this, &MultiDayEventModel::refreshView);

    mSourceModel = sourceModel;
    endResetModel();
}

void MultiDayEventModel::setCalendarFilter(const QSet<QByteArray> &filter)
{
    mFilter = filter;
    setupModel(mStart, mLength, mFilter);
}

QHash<int, QByteArray> MultiDayEventModel::roleNames() const
{
    return {
        {Events, "events"}
    };
}
