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

#include "todomodel.h"

#include <sink/log.h>
#include <sink/query.h>
#include <sink/store.h>
#include <sink/applicationdomaintype.h>

#include <QMetaEnum>

#include <KCalCore/ICalFormat>

#include <entitycache.h>

using namespace Sink;

TodoSourceModel::TodoSourceModel(QObject *parent)
    : QAbstractItemModel(parent),
    mCalendarCache{EntityCache<ApplicationDomain::Calendar>::Ptr::create(QByteArrayList{{ApplicationDomain::Calendar::Color::name}, {ApplicationDomain::Calendar::Name::name}})}
{
    mRefreshTimer.setSingleShot(true);
    QObject::connect(&mRefreshTimer, &QTimer::timeout, this, &TodoSourceModel::updateFromSource);
}

static QList<QByteArray> toList(const QVariant &variant) {
    if (variant.type() == static_cast<QVariant::Type>(QMetaType::QVariantList)) {
        QList<QByteArray> list;
        for (const auto &v : variant.toList()) {
            list.append(v.toByteArray());
        }
        return list;
    }
    return variant.value<QSet<QByteArray>>().toList();
}

void TodoSourceModel::setFilter(const QVariantMap &filter)
{
    const auto account = filter.value("account").toByteArray();
    const auto calendarFilter = toList(filter.value("calendars"));
    using namespace Sink::ApplicationDomain;
    if (calendarFilter.isEmpty()) {
        refreshView();
        return;
    }

    Sink::Query query;
    if (!account.isEmpty()) {
        query.resourceFilter<SinkResource::Account>(account);
    }
    query.filter<Todo::Calendar>(QueryBase::Comparator(QVariant::fromValue(calendarFilter), QueryBase::Comparator::In));

    if (filter.value("doing").toBool()) {
        query.filter<Todo::Status>("INPROCESS");
    }

    query.setFlags(Sink::Query::LiveQuery);
    query.request<Todo::Summary>();
    query.request<Todo::Description>();
    query.request<Todo::StartDate>();
    query.request<Todo::DueDate>();
    query.request<Todo::CompletedDate>();
    query.request<Todo::Status>();
    query.request<Todo::Calendar>();
    query.request<Todo::Ical>();
    query.request<Todo::Priority>();

    mSourceModel = Store::loadModel<ApplicationDomain::Todo>(query);

    QObject::connect(mSourceModel.data(), &QAbstractItemModel::dataChanged, this, &TodoSourceModel::refreshView);
    QObject::connect(mSourceModel.data(), &QAbstractItemModel::layoutChanged, this, &TodoSourceModel::refreshView);
    QObject::connect(mSourceModel.data(), &QAbstractItemModel::modelReset, this, &TodoSourceModel::refreshView);
    QObject::connect(mSourceModel.data(), &QAbstractItemModel::rowsInserted, this, &TodoSourceModel::refreshView);
    QObject::connect(mSourceModel.data(), &QAbstractItemModel::rowsMoved, this, &TodoSourceModel::refreshView);
    QObject::connect(mSourceModel.data(), &QAbstractItemModel::rowsRemoved, this, &TodoSourceModel::refreshView);

    refreshView();
}

void TodoSourceModel::refreshView()
{
    if (!mRefreshTimer.isActive()) {
        //Instant update, but then only refresh every 50ms max.
        updateFromSource();
        mRefreshTimer.start(50);
    }
}

void TodoSourceModel::updateFromSource()
{
    beginResetModel();

    mTodos.clear();

    if (mSourceModel) {
        for (int i = 0; i < mSourceModel->rowCount(); ++i) {
            auto todo = mSourceModel->index(i, 0).data(Sink::Store::DomainObjectRole).value<ApplicationDomain::Todo::Ptr>();
            //Parse the todo
            if(auto icalTodo = KCalCore::ICalFormat().readIncidence(todo->getIcal()).dynamicCast<KCalCore::Todo>()) {
                mTodos.append({icalTodo->dtStart(), icalTodo->dtDue(), icalTodo->completed(), icalTodo, getColor(todo->getCalendar()), getCalendarName(todo->getCalendar()), todo->getStatus(), todo, todo->getPriority()});
            } else {
                SinkWarning() << "Invalid ICal to process, ignoring...";
            }
        }
    }

    endResetModel();
}

QModelIndex TodoSourceModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return {};
    }

    if (!parent.isValid()) {
        return createIndex(row, column);
    }
    return {};
}

QModelIndex TodoSourceModel::parent(const QModelIndex &) const
{
    return {};
}

int TodoSourceModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return mTodos.size();
    }
    return 0;
}

int TodoSourceModel::columnCount(const QModelIndex &) const
{
    return 1;
}

QByteArray TodoSourceModel::getColor(const QByteArray &calendar) const
{
    const auto color = mCalendarCache->getProperty(calendar, {ApplicationDomain::Calendar::Color::name}).toByteArray();
    if (color.isEmpty()) {
        qWarning() << "Failed to get color for calendar " << calendar;
    }
    return color;
}

QString TodoSourceModel::getCalendarName(const QByteArray &calendar) const
{
    const auto name = mCalendarCache->getProperty(calendar, {ApplicationDomain::Calendar::Name::name}).toString();
    if (name.isEmpty()) {
        qWarning() << "Failed to get name for calendar " << calendar;
    }
    return name;
}

QVariant TodoSourceModel::data(const QModelIndex &idx, int role) const
{
    if (!hasIndex(idx.row(), idx.column())) {
        return {};
    }
    auto todo = mTodos.at(idx.row());
    auto icalTodo = todo.incidence;
    switch (role) {
        case Summary:
            return icalTodo->summary();
        case Description:
            return icalTodo->description();
        case StartDate:
            return todo.start;
        case DueDate:
            return todo.due;
        case Date:
            if (todo.status == "COMPLETED") {
                return todo.completed;
            }
            if (todo.due.isValid()) {
                return todo.due;
            }
            return todo.start;
        case CompletedDate:
            return todo.completed;
        case Color:
            return todo.color;
        case Calendar:
            return todo.calendarName;
        case Status:
            return todo.status;
        case Complete:
            return todo.status == "COMPLETED";
        case Doing:
            return todo.status == "INPROCESS";
        case Important:
            return todo.priority == 1;
        case Relevance: {
            int score = 100;
            if (todo.status == "COMPLETED") {
                score -= 100;
            } else {
                //TODO add if from current account
                //Due
                if (todo.due.isValid() && (!todo.start.isValid() || todo.start > QDateTime::currentDateTime())) {
                    score += 10;
                    //Overdue
                    if (todo.due >= QDateTime::currentDateTime()) {
                        score += 50;
                    }
                }
                if (todo.priority == 1) {
                    score += 50;
                }
                if (todo.status == "INPROCESS") {
                    score += 100;
                }
            }
            return score;
        }
        case Todo:
            return QVariant::fromValue(todo.domainObject);
        default:
            SinkWarning() << "Unknown role for todo:" << QMetaEnum::fromType<Roles>().valueToKey(role) << role;
            return {};
    }
}

QHash<int, QByteArray> TodoSourceModel::roleNames() const
{
    return {
        {Summary, "summary"},
        {Description, "description"},
        {StartDate, "startDate"},
        {DueDate, "dueDate"},
        {CompletedDate, "completedDate"},
        {Date, "date"},
        {Color, "color"},
        {Calendar, "calendar"},
        {Status, "status"},
        {Complete, "complete"},
        {Doing, "doing"},
        {Important, "important"},
        {Todo, "domainObject"}
    };
}



TodoModel::TodoModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    sort(0, Qt::DescendingOrder);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setSourceModel(new TodoSourceModel(this));
}

QHash<int, QByteArray> TodoModel::roleNames() const
{
    return sourceModel()->roleNames();
}

void TodoModel::setFilter(const QVariantMap &f)
{
    static_cast<TodoSourceModel*>(sourceModel())->setFilter(f);
}

bool TodoModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    const auto leftScore = left.data(TodoSourceModel::Relevance).toInt();
    const auto rightScore = right.data(TodoSourceModel::Relevance).toInt();
    if (leftScore == rightScore) {
        return left.data(TodoSourceModel::Summary) < right.data(TodoSourceModel::Summary);
    }
    return leftScore < rightScore;
}

bool TodoModel::filterAcceptsRow(int /*sourceRow*/, const QModelIndex &/*sourceParent*/) const
{
    return true;
}
