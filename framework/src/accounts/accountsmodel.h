/*
    Copyright (c) 2016 Christian Mollekopf <mollekopf@kolabsys.com>

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
#include <QObject>
#include <QIdentityProxyModel>
#include <QSharedPointer>
#include <QStringList>

namespace Sink {
    class Query;
}

class KUBE_EXPORT AccountsModel : public QIdentityProxyModel
{
    Q_OBJECT

    Q_PROPERTY (QByteArray accountId READ accountId WRITE setAccountId)
    Q_PROPERTY (QByteArray resourceId READ resourceId WRITE setResourceId)
public:
    enum Status {
        OfflineStatus,
        ConnectedStatus,
        BusyStatus,
        ErrorStatus,
        NoStatus
    };
    Q_ENUMS(Status)

    AccountsModel(QObject *parent = Q_NULLPTR);
    ~AccountsModel();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    enum Roles {
        Name  = Qt::UserRole + 1,
        Icon,
        AccountId,
        Status
    };
    Q_ENUMS(Roles)

    QHash<int, QByteArray> roleNames() const;

    void setAccountId(const QByteArray &id);
    QByteArray accountId() const;

    void setResourceId(const QByteArray &id);
    QByteArray resourceId() const;

private:
    void runQuery(const Sink::Query &query);
    QSharedPointer<QAbstractItemModel> mModel;
};
