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

#include <eventmodel.h>

enum Roles {
    Events = EventModel::LastRole
};

MultiDayEventModel::MultiDayEventModel(QObject *parent)
    : QAbstractItemModel(parent)
{
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
    if (!parent.isValid()) {
        return 1;
    }
    return 0;
}

int MultiDayEventModel::columnCount(const QModelIndex &) const
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
                return {};
            }
            //FIXME set start date of this row
            QDate rowStart = mSourceModel->start();
            auto getStart = [&] (const QDate &start) {
                return qMax(rowStart.daysTo(start), 0ll);
            };

            auto getDuration = [&] (const QDate &start, const QDate &end) {
                return qMax(start.daysTo(end), 1ll);
            };

            QMultiMap<int, QModelIndex> sorted;
            //Sort by duration
            for (int row = 0; row < mSourceModel->rowCount(); row++) {
                const auto srcIdx = mSourceModel->index(row, 0, {});
                //Filter for allday only
                if (!srcIdx.data(EventModel::AllDay).toBool()) {
                    continue;
                }
                sorted.insert(getDuration(srcIdx.data(EventModel::StartTime).toDateTime().date(), srcIdx.data(EventModel::EndTime).toDateTime().date()), srcIdx);
            }

            auto result = QVariantList{};
            auto currentLine = QVariantList{};
            int lastStart = -1;
            int lastDuration = 0;
            for (const auto &srcIdx : sorted) {
                const auto start = getStart(srcIdx.data(EventModel::StartTime).toDateTime().date());
                const auto duration = qMin(getDuration(srcIdx.data(EventModel::StartTime).toDateTime().date(), srcIdx.data(EventModel::EndTime).toDateTime().date()), mPeriodLength - start);
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
                        // qDebug() << "Found intersection " << currentLine;
                        currentLine = {};
                    }
                }
                lastStart = start;
                lastDuration = duration;
            }
            if (!currentLine.isEmpty()) {
                result.append(QVariant::fromValue(currentLine));
            }

            // qDebug() << "Found events " << result;
            return result;
        }
        default:
            Q_ASSERT(false);
            return {};
    }
}

void MultiDayEventModel::setModel(EventModel *model)
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

QHash<int, QByteArray> MultiDayEventModel::roleNames() const
{
    return {
        {Events, "events"}
    };
}
