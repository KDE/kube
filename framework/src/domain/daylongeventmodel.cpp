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

#include "daylongeventmodel.h"

#include <sink/log.h>
#include <sink/query.h>
#include <sink/store.h>

DayLongEventModel::DayLongEventModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    Sink::Query query;
    query.setFlags(Sink::Query::LiveQuery);
    query.request<Event::Summary>();
    query.request<Event::Description>();
    query.request<Event::StartTime>();
    query.request<Event::EndTime>();

    query.filter<Event::AllDay>(true);

    mModel = Sink::Store::loadModel<Event>(query);

    setSourceModel(mModel.data());
}

QHash<int, QByteArray> DayLongEventModel::roleNames() const
{
    return {
        {Summary, "summary"},
        {Description, "description"},
        {StartDate, "starts"},
        {Duration, "duration"},
    };
}

QVariant DayLongEventModel::data(const QModelIndex &idx, int role) const
{
    auto srcIdx = mapToSource(idx);
    auto event  = srcIdx.data(Sink::Store::DomainObjectRole).value<Event::Ptr>();

    switch (role) {
        case Summary:
            return event->getSummary();
        case Description:
            return event->getDescription();
        case StartDate: {
            auto dayIndex = mPeriodStart.daysTo(event->getStartTime().date());
            if (dayIndex < 0) {
                return 0;
            }

            return dayIndex;
        }
        case Duration:
            return event->getStartTime().date().daysTo(event->getEndTime().date());
    }

    return QSortFilterProxyModel::data(idx, role);
}

bool DayLongEventModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    auto idx   = sourceModel()->index(sourceRow, 0, sourceParent);
    auto event = idx.data(Sink::Store::DomainObjectRole).value<Event::Ptr>();

    auto eventStart = event->getStartTime().date();
    auto eventEnd   = event->getEndTime().date();

    if (!eventStart.isValid() || !eventEnd.isValid()) {
        SinkWarning() << "Invalid date in the event model, ignoring...";
        return false;
    }

    return eventStart < mPeriodStart.addDays(mPeriodLength) && eventEnd >= mPeriodStart;
}

QDate DayLongEventModel::periodStart() const
{
    return mPeriodStart;
}

void DayLongEventModel::setPeriodStart(const QDate &start)
{
    if (!start.isValid()) {
        SinkWarning() << "Passed an invalid starting date in setPeriodStart, ignoring...";
        return;
    }

    mPeriodStart = start;
}

void DayLongEventModel::setPeriodStart(const QVariant &start)
{
    setPeriodStart(start.toDate());
}

int DayLongEventModel::periodLength() const
{
    return mPeriodLength;
}

void DayLongEventModel::setPeriodLength(int length)
{
    mPeriodLength = length;
}
