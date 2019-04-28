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
#include <QList>
#include <QSet>
#include <QSharedPointer>
#include <QTimer>
#include <QDateTime>

namespace KCalCore {
    class MemoryCalendar;
    class Incidence;
}
namespace Sink {
    namespace ApplicationDomain {
        struct Event;
    }
}
class EntityCacheInterface;

/**
 * Loads all event occurrences within the given period and matching the given filter.
 *
 * Recurrences are expanded
 */
class KUBE_EXPORT EventOccurrenceModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(QDate start READ start WRITE setStart)
    Q_PROPERTY(int length READ length WRITE setLength)
    Q_PROPERTY(QSet<QByteArray> calendarFilter WRITE setCalendarFilter)
    Q_PROPERTY(QVariantMap filter WRITE setFilter)

public:
    enum Roles {
        Summary = Qt::UserRole + 1,
        Description,
        StartTime,
        EndTime,
        Color,
        AllDay,
        Event,
        EventOccurrence,
        LastRole
    };
    Q_ENUM(Roles);
    EventOccurrenceModel(QObject *parent = nullptr);
    ~EventOccurrenceModel() = default;

    QModelIndex index(int row, int column, const QModelIndex &parent = {}) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    void updateQuery(const QDate &start, const QDate &end, const QSet<QByteArray> &calendarFilter);

    void setStart(const QDate &);
    QDate start() const;
    void setLength(int);
    int length() const;
    void setCalendarFilter(const QSet<QByteArray> &);
    void setFilter(const QVariantMap &);

    struct Occurrence {
        QDateTime start;
        QDateTime end;
        QSharedPointer<KCalCore::Incidence> incidence;
        QByteArray color;
        bool allDay;
        QSharedPointer<Sink::ApplicationDomain::Event> domainObject;
    };


private:
    void updateQuery();

    void refreshView();
    void updateFromSource();
    QByteArray getColor(const QByteArray &calendar) const;

    QSharedPointer<QAbstractItemModel> mSourceModel;
    QSet<QByteArray> mCalendarFilter;
    QDate mStart;
    QDate mEnd;
    int mLength{0};
    QSharedPointer<EntityCacheInterface> mCalendarCache;

    QSharedPointer<KCalCore::MemoryCalendar> mCalendar;
    QTimer mRefreshTimer;

    QList<Occurrence> mEvents;
    QVariantMap mFilter;
};

Q_DECLARE_METATYPE(EventOccurrenceModel::Occurrence);
