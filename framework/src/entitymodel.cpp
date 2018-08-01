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

#include "entitymodel.h"

#include <sink/store.h>
#include <sink/log.h>

using namespace Sink;
using namespace Sink::ApplicationDomain;

EntityModel::EntityModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    sort(0, Qt::AscendingOrder);
}

EntityModel::~EntityModel()
{

}

QHash< int, QByteArray > EntityModel::roleNames() const
{
    return mRoleNames;
}

QVariant EntityModel::data(const QModelIndex &idx, int role) const
{
    auto srcIdx = mapToSource(idx);
    auto entity = srcIdx.data(Sink::Store::DomainObjectBaseRole).value<Sink::ApplicationDomain::ApplicationDomainType::Ptr>();

    const auto roleName = mRoleNames.value(role);
    if (roleName == "identifier") {
        return entity->identifier();
    } else if (roleName == "object") {
        return QVariant::fromValue(entity);
    } else {
        return entity->getProperty(roleName);
    }
}

void EntityModel::runQuery(const Query &query)
{
    if (mType == "calendar") {
        mModel = Store::loadModel<Calendar>(query);
    } else {
        qWarning() << "Type not supported " << mType;
    }
    setSourceModel(mModel.data());
}

void EntityModel::updateQuery()
{
    if (mType.isEmpty() || mRoles.isEmpty()) {
        return;
    }

    Query query;
    if (!mAccountId.isEmpty()) {
        query.resourceFilter<SinkResource::Account>(mAccountId.toUtf8());
    }
    query.setFlags(Sink::Query::LiveQuery | Sink::Query::UpdateStatus);

    for (const auto &property: mRoles.keys()) {
        query.requestedProperties << property;
    }
    runQuery(query);
}

void EntityModel::setAccountId(const QString &accountId)
{

    //Get all folders of an account
    mAccountId = accountId;
    updateQuery();
}

QString EntityModel::accountId() const
{
    return {};
}

void EntityModel::setType(const QString &type)
{
    mType = type;
    updateQuery();
}

QString EntityModel::type() const
{
    return {};
}

void EntityModel::setRoles(const QStringList &roles)
{
    mRoleNames.clear();
    int role = Qt::UserRole + 1;
    mRoleNames.insert(role++, "identifier");
    mRoleNames.insert(role++, "object");
    for (int i = 0; i < roles.size(); i++) {
        mRoleNames.insert(role++, roles.at(i).toLatin1());
    }
    mRoles.clear();
    for (const auto &r : mRoleNames.keys()) {
        mRoles.insert(mRoleNames.value(r), r);
    }
    updateQuery();
}

QStringList EntityModel::roles() const
{
    // return mRoleNames.values();
    return {};
}

void EntityModel::setFilter(const QVariantMap &)
{
    //TODO
}

QVariantMap EntityModel::filter() const
{
    return {};
}
