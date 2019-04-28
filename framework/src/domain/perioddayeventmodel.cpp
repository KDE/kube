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

enum Roles {
    Events = EventOccurrenceModel::LastRole,
    Date
};

PeriodDayEventModel::PeriodDayEventModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

void PeriodDayEventModel::setModel(EventOccurrenceModel *model)
{
    beginResetModel();
    mSourceModel = model;
    auto resetModel = [this] {
        beginResetModel();
        endResetModel();
    };
    QObject::connect(model, &QAbstractItemModel::dataChanged, this, resetModel);
    QObject::connect(model, &QAbstractItemModel::layoutChanged, this, resetModel);
    QObject::connect(model, &QAbstractItemModel::modelReset, this, resetModel);
    QObject::connect(model, &QAbstractItemModel::rowsInserted, this, resetModel);
    QObject::connect(model, &QAbstractItemModel::rowsMoved, this, resetModel);
    QObject::connect(model, &QAbstractItemModel::rowsRemoved, this, resetModel);
    endResetModel();
}

QModelIndex PeriodDayEventModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return {};
    }

    if (!parent.isValid()) {
        // Asking for a day
        return createIndex(row, column, DAY_ID);
    }

    return {};
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
    if (!parent.isValid() && mSourceModel) {
        return mSourceModel->length();
    }
    return 0;
}

int PeriodDayEventModel::columnCount(const QModelIndex &) const
{
    return 1;
}

QDateTime PeriodDayEventModel::getStartTimeOfDay(const QDateTime &dateTime, const QDate &today) const
{
    if (dateTime.date() != today) {
        return QDateTime{today, QTime{0,0}};
    }
    return dateTime;
}

QDateTime PeriodDayEventModel::getEndTimeOfDay(const QDateTime &dateTime, const QDate &today) const
{
    if (dateTime.date() != today) {
        return QDateTime{today, QTime{23, 59, 59}};
    }
    return dateTime;
}

QVariant PeriodDayEventModel::data(const QModelIndex &idx, int role) const
{
    if (!mSourceModel) {
        return {};
    }
    const auto dayOffset = idx.row();
    const QDate startDate = mSourceModel->start();
    const auto today = startDate.addDays(dayOffset);
    switch (role) {
        case Date:
            return today;
        case Events: {
            auto result = QVariantList{};

            QMultiMap<QTime, QModelIndex> sorted;
            for (int row = 0; row < mSourceModel->rowCount(); row++) {
                const auto srcIdx = mSourceModel->index(row, 0, {});
                if (srcIdx.data(EventOccurrenceModel::AllDay).toBool()) {
                    continue;
                }
                const auto start = srcIdx.data(EventOccurrenceModel::StartTime).toDateTime();
                const auto end = srcIdx.data(EventOccurrenceModel::EndTime).toDateTime();
                if (end.date() < today || start.date() > today) {
                    continue;
                }
                sorted.insert(srcIdx.data(EventOccurrenceModel::StartTime).toDateTime().time(), srcIdx);
            }

            QMap<QTime, int> indentationStack;
            for (auto it = sorted.begin(); it != sorted.end(); it++) {
                // auto eventid = index(it.value(), 0, idx);
                const auto srcIdx = it.value();

                const auto start = getStartTimeOfDay(srcIdx.data(EventOccurrenceModel::StartTime).toDateTime(), today);
                const auto startTime = start.time();
                const auto end = getEndTimeOfDay(srcIdx.data(EventOccurrenceModel::EndTime).toDateTime(), today);
                auto endTime = end.time();
                if (!endTime.isValid()) {
                    //Even without duration we still take some space visually
                    endTime = startTime.addSecs(60 * 20);
                }
                const auto duration = qRound(startTime.secsTo(endTime) / 3600.0);
                SinkTrace() << "Appending event:" << srcIdx.data(EventOccurrenceModel::Summary) << start << end;

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
                    {"text", srcIdx.data(EventOccurrenceModel::Summary)},
                    {"description", srcIdx.data(EventOccurrenceModel::Description)},
                    {"starts", startTime.hour() + startTime.minute() / 60.},
                    {"duration", duration},
                    {"color", srcIdx.data(EventOccurrenceModel::Color)},
                    {"indentation", indentation},
                    {"occurrenceDate", srcIdx.data(EventOccurrenceModel::StartTime)},
                    {"event", srcIdx.data(EventOccurrenceModel::Event)}
                });
            }

            return result;
        }
        default:
            Q_ASSERT(false);
            return {};
    }
}

QHash<int, QByteArray> PeriodDayEventModel::roleNames() const
{
    return {
        {Events, "events"},
        {Date, "date"}
    };
}
