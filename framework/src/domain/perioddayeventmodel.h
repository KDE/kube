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

#pragma once
#include "kube_export.h"
#include <sink/applicationdomaintype.h>

#include <QAbstractItemModel>
#include <QList>
#include <QSharedPointer>
#include <QVector>

#include <limits>

// Facility used to get a restricted period into a Sink model comprised of
// events, partitioned according to the day the events take place.
//
// Day-long events are filtered out.
//
// Model Format
// ============
//
// Day 0
//  |--- Event 0 starting at `periodStart + 0d`
//  |--- Event 1 starting at `periodStart + 0d`
//  '--- Event 2 starting at `periodStart + 0d`
// Day 1
//  '--- Event 0 starting at `periodStart + 1d`
// Day 2
// Day 3
//  |--- Event 0 starting at `periodStart + 3d`
//  '--- Event 1 starting at `periodStart + 3d`
// Day 4
//   ⋮
//
// Implementation notes
// ====================
//
// On the model side
// -----------------
//
// Columns are never used.
//
// Top-level items just contains the ".events" attribute, and their rows
// correspond to their offset compared to the start of the period (in number of
// days). In that case the internalId contains DAY_ID.
//
// Direct children are events, and their rows corresponds to their index in
// their partition. In that case no internalId / internalPointer is used.
//
// Internally:
// -----------
//
// On construction and on dataChanged, all events are processed and partitioned
// in partitionedEvents:
//
// QVector< QList<QSharedPointer<Event> >
// ~~~~~~~  ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//    |                |
//    |                '--- List of event pointers for that day
//    '--- Partition / day
//
class KUBE_EXPORT PeriodDayEventModel : public QAbstractItemModel
{
    Q_OBJECT

    Q_PROPERTY(QVariant start READ periodStart WRITE setPeriodStart)
    Q_PROPERTY(int length READ periodLength WRITE setPeriodLength)

public:
    using Event = Sink::ApplicationDomain::Event;

    enum Roles
    {
        Events = Qt::UserRole + 1,
        Summary,
        Description,
        StartTime,
        Duration,
    };
    Q_ENUM(Roles);
    PeriodDayEventModel(QObject *parent = nullptr);
    ~PeriodDayEventModel() = default;

    QModelIndex index(int row, int column, const QModelIndex &parent = {}) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    QHash<int, QByteArray> roleNames() const override;

    QDate periodStart() const;
    void setPeriodStart(const QDate &);
    void setPeriodStart(const QVariant &);
    int periodLength() const;
    void setPeriodLength(int);

private:
    void updateQuery();
    void partitionData();

    int bucketOf(const QDate &candidate) const;

    QDate mPeriodStart;
    int mPeriodLength = 7;

    QSharedPointer<QAbstractItemModel> eventModel;
    QVector<QList<QSharedPointer<Event>>> partitionedEvents;

    static const constexpr quintptr DAY_ID = std::numeric_limits<quintptr>::max();
};
