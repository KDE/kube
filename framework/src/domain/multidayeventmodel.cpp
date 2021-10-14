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

#include <eventoccurrencemodel.h>

enum Roles {
    Events = EventOccurrenceModel::LastRole,
    WeekStartDate
};

MultiDayEventModel::MultiDayEventModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    mRefreshTimer.setSingleShot(true);
    QObject::connect(&mRefreshTimer, &QTimer::timeout, this, &MultiDayEventModel::reset);
}

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

QModelIndex MultiDayEventModel::parent(const QModelIndex &) const
{
    return {};
}

int MultiDayEventModel::rowCount(const QModelIndex &parent) const
{
    //Number of weeks
    if (!parent.isValid() && mSourceModel) {
        return qMax(mSourceModel->length() / 7, 1);
    }
    return 0;
}

int MultiDayEventModel::columnCount(const QModelIndex &) const
{
    return 1;
}


static long long getDuration(const QDate &start, const QDate &end)
{
    return qMax(start.daysTo(end) + 1, 1ll);
}

// We first sort all occurences so we get all-day first (sorted by duration),
// and then the rest sorted by start-date.
QList<QModelIndex> MultiDayEventModel::sortedEventsFromSourceModel(const QDate &rowStart) const
{
    const auto rowEnd = rowStart.addDays(7);
    QList<QModelIndex> sorted;
    sorted.reserve(mSourceModel->rowCount());
    for (int row = 0; row < mSourceModel->rowCount(); row++) {
        const auto srcIdx = mSourceModel->index(row, 0, {});
        const auto start = srcIdx.data(EventOccurrenceModel::StartTime).toDateTime().date();
        const auto end = srcIdx.data(EventOccurrenceModel::EndTime).toDateTime().date();
        //Skip events not part of the week
        if (end < rowStart || start > rowEnd) {
            // qWarning() << "Skipping because not part of this week";
            continue;
        }
        // qWarning() << "found " << srcIdx.data(EventOccurrenceModel::StartTime).toDateTime() << srcIdx.data(EventOccurrenceModel::Summary).toString();
        sorted.append(srcIdx);
    }
    std::sort(sorted.begin(), sorted.end(), [&] (const QModelIndex &left, const QModelIndex &right) {
        //All-day first, sorted by duration (in the hope that we can fit multiple on the same line)
        const auto leftAllDay = left.data(EventOccurrenceModel::AllDay).toBool();
        const auto rightAllDay = right.data(EventOccurrenceModel::AllDay).toBool();
        if (leftAllDay && !rightAllDay) {
            return true;
        }
        if (!leftAllDay && rightAllDay) {
            return false;
        }
        if (leftAllDay && rightAllDay) {
            const auto leftDuration = getDuration(left.data(EventOccurrenceModel::StartTime).toDateTime().date(), left.data(EventOccurrenceModel::EndTime).toDateTime().date());
            const auto rightDuration = getDuration(right.data(EventOccurrenceModel::StartTime).toDateTime().date(), right.data(EventOccurrenceModel::EndTime).toDateTime().date());
            return leftDuration < rightDuration;
        }
        //The rest sorted by start date
        return left.data(EventOccurrenceModel::StartTime).toDateTime() < right.data(EventOccurrenceModel::StartTime).toDateTime();
    });
    return sorted;
}

/*
* Layout the lines:
*
* The line grouping algorithm then always picks the first event,
* and tries to add more to the same line.
*
* We never mix all-day and non-all day, and otherwise try to fit as much as possible
* on the same line. Same day time-order should be preserved because of the sorting.
*/
QVariantList MultiDayEventModel::layoutLines(const QDate &rowStart) const
{
    auto getStart = [&rowStart] (const QDate &start) {
        return qMax(rowStart.daysTo(start), 0ll);
    };

    QList<QModelIndex> sorted = sortedEventsFromSourceModel(rowStart);

    // for (const auto &srcIdx : sorted) {
    //     qWarning() << "sorted " << srcIdx.data(EventOccurrenceModel::StartTime).toDateTime() << srcIdx.data(EventOccurrenceModel::Summary).toString() << srcIdx.data(EventOccurrenceModel::AllDay).toBool();
    // }

    auto result = QVariantList{};
    while (!sorted.isEmpty()) {
        const auto srcIdx = sorted.takeFirst();
        const auto startDate = srcIdx.data(EventOccurrenceModel::StartTime).toDateTime();
        const auto start = getStart(startDate.date());
        const auto duration = qMin(getDuration(startDate.date(), srcIdx.data(EventOccurrenceModel::EndTime).toDateTime().date()), mPeriodLength - start);
        // qWarning() << "First of line " << srcIdx.data(EventOccurrenceModel::StartTime).toDateTime() << duration << srcIdx.data(EventOccurrenceModel::Summary).toString();
        auto currentLine = QVariantList{};

        auto addToLine = [&currentLine] (const QModelIndex &idx, int start, int duration) {
            currentLine.append(QVariantMap{
                {"text", idx.data(EventOccurrenceModel::Summary)},
                {"description", idx.data(EventOccurrenceModel::Description)},
                {"starts", start},
                {"duration", duration},
                {"color", idx.data(EventOccurrenceModel::Color)},
                {"eventOccurrence", idx.data(EventOccurrenceModel::EventOccurrence)}
            });
        };

        //Add first event of line
        addToLine(srcIdx, start, duration);
        const bool allDayLine = srcIdx.data(EventOccurrenceModel::AllDay).toBool();

        //Fill line with events that fit
        int lastStart = start;
        int lastDuration = duration;
        auto doesIntersect = [&] (int start, int end) {
            const auto lastEnd = lastStart + lastDuration;
            if (((start <= lastStart) && (end >= lastStart)) ||
                ((start < lastEnd) && (end > lastStart))) {
                // qWarning() << "Found intersection " << start << end;
                return true;
            }
            return false;
        };

        for (auto it = sorted.begin(); it != sorted.end();) {
            const auto idx = *it;
            const auto start = getStart(idx.data(EventOccurrenceModel::StartTime).toDateTime().date());
            const auto duration = qMin(getDuration(idx.data(EventOccurrenceModel::StartTime).toDateTime().date(), idx.data(EventOccurrenceModel::EndTime).toDateTime().date()), mPeriodLength - start);
            const auto end = start + duration;

            // qWarning() << "Checking " << idx.data(EventOccurrenceModel::StartTime).toDateTime() << duration << idx.data(EventOccurrenceModel::Summary).toString();
            //Avoid mixing all-day and other events
            if (allDayLine && !idx.data(EventOccurrenceModel::AllDay).toBool()) {
                break;
            }

            if (doesIntersect(start, end)) {
                it++;
            } else {
                addToLine(idx, start, duration);
                lastStart = start;
                lastDuration = duration;
                it = sorted.erase(it);
            }
        }
        // qWarning() << "Appending line " << currentLine;
        result.append(QVariant::fromValue(currentLine));
    }
    return result;
}

QVariant MultiDayEventModel::data(const QModelIndex &idx, int role) const
{
    if (!hasIndex(idx.row(), idx.column())) {
        return {};
    }
    if (!mSourceModel) {
        return {};
    }
    const auto rowStart = mSourceModel->start().addDays(idx.row() * 7);
    switch (role) {
        case WeekStartDate:
            return rowStart;
        case Events:
            return layoutLines(rowStart);
        default:
            Q_ASSERT(false);
            return {};
    }
}

void MultiDayEventModel::reset()
{
    beginResetModel();
    endResetModel();
}

void MultiDayEventModel::setModel(EventOccurrenceModel *model)
{
    beginResetModel();
    mSourceModel = model;
    auto resetModel = [this] {
        if (!mRefreshTimer.isActive()) {
            mRefreshTimer.start(50);
            reset();
        }
    };
    QObject::connect(model, &QAbstractItemModel::dataChanged, this, resetModel);
    QObject::connect(model, &QAbstractItemModel::layoutChanged, this, resetModel);
    QObject::connect(model, &QAbstractItemModel::modelReset, this, resetModel);
    QObject::connect(model, &QAbstractItemModel::rowsInserted, this, resetModel);
    QObject::connect(model, &QAbstractItemModel::rowsMoved, this, resetModel);
    QObject::connect(model, &QAbstractItemModel::rowsRemoved, this, resetModel);
    endResetModel();
}

QHash<int, QByteArray> MultiDayEventModel::roleNames() const
{
    return {
        {Events, "events"},
        {WeekStartDate, "weekStartDate"}
    };
}
