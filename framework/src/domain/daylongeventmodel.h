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

#include <QList>
#include <QSharedPointer>
#include <QSortFilterProxyModel>
#include <QVector>

class EntityCacheInterface;
class KUBE_EXPORT DayLongEventModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(QVariant start READ periodStart WRITE setPeriodStart)
    Q_PROPERTY(int length READ periodLength WRITE setPeriodLength)
    Q_PROPERTY(QSet<QByteArray> calendarFilter READ calendarFilter WRITE setCalendarFilter)

public:
    using Event = Sink::ApplicationDomain::Event;
    using Calendar = Sink::ApplicationDomain::Calendar;

    enum Roles
    {
        Summary = Qt::UserRole + 1,
        Description,
        StartDate,
        Duration,
        Color
    };
    Q_ENUM(Roles);

    DayLongEventModel(QObject *parent = nullptr);
    ~DayLongEventModel() = default;

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    QDate periodStart() const;
    void setPeriodStart(const QDate &);
    void setPeriodStart(const QVariant &);
    int periodLength() const;
    void setPeriodLength(int);

    QSet<QByteArray> calendarFilter() const;
    void setCalendarFilter(const QSet<QByteArray> &);

private:
    QByteArray getColor(const QByteArray &calendar) const;

    QSharedPointer<QAbstractItemModel> mModel;
    QSharedPointer<EntityCacheInterface> mCalendarCache;
    QSet<QByteArray> mCalendarFilter;

    QDate mPeriodStart;
    int mPeriodLength = 7;
};
