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
#include "identitiesmodel.h"
#include <sink/store.h>
#include <sink/log.h>

using namespace Sink;

IdentitiesModel::IdentitiesModel(QObject *parent) : QIdentityProxyModel()
{
    Sink::Query query;
    query.setFlags(Sink::Query::LiveQuery);
    query.request<Sink::ApplicationDomain::Identity::Name>()
        .request<Sink::ApplicationDomain::Identity::Address>()
        .request<Sink::ApplicationDomain::Identity::Account>();
    runQuery(query);
}

IdentitiesModel::~IdentitiesModel()
{

}

QHash< int, QByteArray > IdentitiesModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[Name] = "name";
    roles[Username] = "username";
    roles[Address] = "address";
    roles[IdentityId] = "identityId";
    roles[AccountId] = "accountId";
    roles[AccountName] = "accountName";
    roles[AccountIcon] = "accountIcon";
    roles[DisplayName] = "displayName";

    return roles;
}

QVariant IdentitiesModel::data(const QModelIndex &idx, int role) const
{
    auto srcIdx = mapToSource(idx);
    switch (role) {
        case Name:
            return srcIdx.data(Sink::Store::DomainObjectRole).value<Sink::ApplicationDomain::Identity::Ptr>()->getName();
        case Username:
            return srcIdx.data(Sink::Store::DomainObjectRole).value<Sink::ApplicationDomain::Identity::Ptr>()->getName();
        case Address:
            return srcIdx.data(Sink::Store::DomainObjectRole).value<Sink::ApplicationDomain::Identity::Ptr>()->getAddress();
        case IdentityId:
            return srcIdx.data(Sink::Store::DomainObjectRole).value<Sink::ApplicationDomain::Identity::Ptr>()->identifier();
        case AccountId:
            return srcIdx.data(Sink::Store::DomainObjectRole).value<Sink::ApplicationDomain::Identity::Ptr>()->getAccount();
        case AccountName: {
            const auto accountId = srcIdx.data(Sink::Store::DomainObjectRole).value<Sink::ApplicationDomain::Identity::Ptr>()->getAccount();
            return mAccountNames.value(accountId);
        }
        case AccountIcon: {
            const auto accountId = srcIdx.data(Sink::Store::DomainObjectRole).value<Sink::ApplicationDomain::Identity::Ptr>()->getAccount();
            return mAccountIcons.value(accountId);
        }
        case DisplayName: {
            return data(idx, AccountName).toString() + ": " + data(idx, Username).toString() + ", " + data(idx, Address).toString();
        }
    }
    return QIdentityProxyModel::data(idx, role);
}

void IdentitiesModel::runQuery(const Sink::Query &query)
{
    using namespace Sink::ApplicationDomain;
    mModel = Sink::Store::loadModel<Identity>(query);
    setSourceModel(mModel.data());

    Sink::Store::fetchAll<SinkAccount>(Sink::Query{}.request<SinkAccount::Icon>().request<SinkAccount::Name>())
        .then([this](const QList<SinkAccount::Ptr> &accounts) {
            for (const auto &account : accounts) {
                mAccountNames.insert(account->identifier(), account->getName());
                mAccountIcons.insert(account->identifier(), account->getIcon());
            }
            emit layoutChanged();
        }).exec();
}
