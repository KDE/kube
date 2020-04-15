/*
    Copyright (c) 2018 Christian Mollekopf <mollekopf@kolabsys.com>

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
#include <QSortFilterProxyModel>
#include <QList>
#include <QSet>
#include <QSharedPointer>
#include <QTimer>
#include <QDateTime>

namespace KCalCore {
    class Incidence;
}
namespace Sink {
    namespace ApplicationDomain {
        struct Todo;
    }
}
class EntityCacheInterface;

class KUBE_EXPORT TodoSourceModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum Roles {
        Summary = Qt::UserRole + 1,
        Description,
        StartDate,
        DueDate,
        CompletedDate,
        Date,
        Color,
        Calendar,
        Status,
        Complete,
        Doing,
        Important,
        Relevance,
        Todo,
        LastRole
    };
    Q_ENUM(Roles);

    TodoSourceModel(QObject *parent = nullptr);
    ~TodoSourceModel() = default;

    QModelIndex index(int row, int column, const QModelIndex &parent = {}) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setFilter(const QVariantMap &);

private:
    void refreshView();
    void updateFromSource();
    QByteArray getColor(const QByteArray &calendar) const;
    QString getCalendarName(const QByteArray &calendar) const;

    QSharedPointer<QAbstractItemModel> mSourceModel;
    QSet<QByteArray> mCalendarFilter;
    QSharedPointer<EntityCacheInterface> mCalendarCache;

    QTimer mRefreshTimer;

    struct Occurrence {
        QDateTime start;
        QDateTime due;
        QDateTime completed;
        QSharedPointer<KCalCore::Incidence> incidence;
        QByteArray color;
        QString calendarName;
        QString status;
        QSharedPointer<Sink::ApplicationDomain::Todo> domainObject;
        int priority;
    };

    QList<Occurrence> mTodos;
};

class KUBE_EXPORT TodoModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap filter WRITE setFilter)

public:
    TodoModel(QObject *parent = nullptr);
    ~TodoModel() = default;

    QHash<int, QByteArray> roleNames() const override;

    void setFilter(const QVariantMap &);

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const Q_DECL_OVERRIDE;
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const Q_DECL_OVERRIDE;
};
