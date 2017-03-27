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
#include "accountsmodel.h"
#include <sink/store.h>

using namespace Sink;
using namespace Sink::ApplicationDomain;

AccountsModel::AccountsModel(QObject *parent) : QIdentityProxyModel()
{
    Sink::Query query;
    query.setFlags(Query::LiveQuery);
    query.request<SinkAccount::Name>();
    query.request<SinkAccount::Icon>();
    query.request<SinkAccount::Status>();
    runQuery(query);
}

AccountsModel::~AccountsModel()
{

}

QHash< int, QByteArray > AccountsModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[Name] = "name";
    roles[Icon] = "icon";
    roles[AccountId] = "accountId";
    roles[Status] = "status";

    return roles;
}

QVariant AccountsModel::data(const QModelIndex &idx, int role) const
{
    auto srcIdx = mapToSource(idx);
    auto account = srcIdx.data(Sink::Store::DomainObjectRole).value<SinkAccount::Ptr>();
    switch (role) {
        case Name:
            return account->getName();
        case Icon:
            return account->getIcon();
        case AccountId:
            return account->identifier();
        case Status:
            switch (account->getStatus()) {
                case Sink::ApplicationDomain::ErrorStatus:
                    return ErrorStatus;
                case Sink::ApplicationDomain::BusyStatus:
                    return BusyStatus;
                case Sink::ApplicationDomain::ConnectedStatus:
                    return ConnectedStatus;
            }
            return OfflineStatus;
    }
    return QIdentityProxyModel::data(idx, role);
}

void AccountsModel::runQuery(const Sink::Query &query)
{
    mModel = Sink::Store::loadModel<SinkAccount>(query);
    setSourceModel(mModel.data());
}

void AccountsModel::setAccountId(const QByteArray &accountId)
{
    qWarning() << "Setting account id" << accountId;
    //Get all folders of an account
    Sink::Query query;
    query.filter(accountId);
    query.setFlags(Query::LiveQuery);
    query.request<SinkAccount::Name>();
    query.request<SinkAccount::Icon>();
    query.request<SinkAccount::Status>();
    runQuery(query);
}

QByteArray AccountsModel::accountId() const
{
    return {};
}
