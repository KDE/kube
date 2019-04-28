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
#include "eventoccurrencemodel.h"

namespace KCalCore {
    class MemoryCalendar;
    class Incidence;
}
class EntityCacheInterface;

/**
 * Each toplevel index represents a week.
 * The "events" roles provides a list of lists, where each list represents a visual line,
 * containing a number of events to display.
 */
class KUBE_EXPORT MultiDayEventModel : public QAbstractItemModel
{
    Q_OBJECT

    Q_PROPERTY(EventOccurrenceModel* model WRITE setModel)

public:
    MultiDayEventModel(QObject *parent = nullptr);
    ~MultiDayEventModel() = default;

    QModelIndex index(int row, int column, const QModelIndex &parent = {}) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    QHash<int, QByteArray> roleNames() const override;

    void setModel(EventOccurrenceModel *model);
private:
    EventOccurrenceModel *mSourceModel{nullptr};
    int mPeriodLength{7};
};
