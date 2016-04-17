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

IdentitiesModel::IdentitiesModel(QObject *parent) : QIdentityProxyModel()
{
    Sink::Query query;
    query.liveQuery = true;
    query.requestedProperties << "name" << "username" << "address" << "account";
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
            return srcIdx.sibling(srcIdx.row(), 0).data(Qt::DisplayRole).toString();
        case Username:
            return srcIdx.sibling(srcIdx.row(), 1).data(Qt::DisplayRole).toString();
        case Address:
            return srcIdx.sibling(srcIdx.row(), 2).data(Qt::DisplayRole).toString();
        case IdentityId:
            return srcIdx.data(Sink::Store::DomainObjectBaseRole).value<Sink::ApplicationDomain::ApplicationDomainType::Ptr>()->identifier();
        case AccountId:
            return srcIdx.data(Sink::Store::DomainObjectBaseRole).value<Sink::ApplicationDomain::ApplicationDomainType::Ptr>()->getProperty("account").toByteArray();
        case AccountName: {
            const auto accountId = srcIdx.sibling(srcIdx.row(), 3).data(Qt::DisplayRole).toByteArray();
            return mAccountNames.value(accountId);
        }
        case AccountIcon: {
            const auto accountId = srcIdx.sibling(srcIdx.row(), 3).data(Qt::DisplayRole).toByteArray();
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
    mModel = Sink::Store::loadModel<Sink::ApplicationDomain::Identity>(query);
    setSourceModel(mModel.data());

    Sink::Store::fetchAll<Sink::ApplicationDomain::SinkAccount>(Sink::Query())
        .then<void, QList<Sink::ApplicationDomain::SinkAccount::Ptr> >([this](const QList<Sink::ApplicationDomain::SinkAccount::Ptr> &accounts) {
            for (const auto &account : accounts) {
                mAccountNames.insert(account->identifier(), account->getProperty("name").toString());
                mAccountIcons.insert(account->identifier(), account->getProperty("icon").toString());
            }
            emit layoutChanged();
        }).exec();
}
