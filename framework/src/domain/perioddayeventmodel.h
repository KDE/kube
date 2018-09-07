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

#include <QAbstractItemModel>
#include <QSharedPointer>
#include <QDate>
#include <QVariant>
#include <QSet>
#include <limits>

class KUBE_EXPORT PeriodDayEventModel : public QAbstractItemModel
{
    Q_OBJECT

    Q_PROPERTY(QVariant start READ periodStart WRITE setPeriodStart)
    Q_PROPERTY(int length READ periodLength WRITE setPeriodLength)
    Q_PROPERTY(QSet<QByteArray> calendarFilter READ calendarFilter WRITE setCalendarFilter)

public:
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

    QSet<QByteArray> calendarFilter() const;
    void setCalendarFilter(const QSet<QByteArray> &);

private:
    void updateQuery();
    void refreshView();
    QDateTime getStartTimeOfDay(const QDateTime &dateTime, const QDate &today) const;
    QDateTime getEndTimeOfDay(const QDateTime &dateTime, const QDate &today) const;

    QDate mPeriodStart;
    int mPeriodLength = 7;

    QSharedPointer<QAbstractItemModel> mSourceModel;
    QSet<QByteArray> mCalendarFilter;

    static const constexpr quintptr DAY_ID = std::numeric_limits<quintptr>::max();
};
