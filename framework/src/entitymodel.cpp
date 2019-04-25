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

enum Roles {
    IdRole = Qt::UserRole + 1,
    ObjectRole,
    LastRole
};

EntityModel::EntityModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setSortCaseSensitivity(Qt::CaseInsensitive);
}

EntityModel::~EntityModel()
{

}

QHash<int, QByteArray> EntityModel::roleNames() const
{
    return mRoleNames;
}

QVariant EntityModel::data(const QModelIndex &idx, int role) const
{
    auto srcIdx = mapToSource(idx);
    if (auto entity = srcIdx.data(Sink::Store::DomainObjectBaseRole).value<Sink::ApplicationDomain::ApplicationDomainType::Ptr>()) {
        const auto roleName = mRoleNames.value(role);
        if (roleName == "identifier") {
            return entity->identifier();
        } else if (roleName == "object") {
            return QVariant::fromValue(entity);
        } else {
            return entity->getProperty(roleName);
        }
    }
    //We can run into this when passing in an invalid index
    return {};
}

bool EntityModel::lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const
{
    auto left = sourceLeft.data(Sink::Store::DomainObjectBaseRole).value<Sink::ApplicationDomain::ApplicationDomainType::Ptr>();
    auto right = sourceRight.data(Sink::Store::DomainObjectBaseRole).value<Sink::ApplicationDomain::ApplicationDomainType::Ptr>();
    const auto leftProperty =  left->getProperty(mSortRole.toUtf8()).toString();
    const auto rightProperty =  right->getProperty(mSortRole.toUtf8()).toString();
    if (leftProperty == rightProperty) {
        return left->identifier() < right->identifier();
    }
    return leftProperty < rightProperty;
}

void EntityModel::runQuery(const Query &query)
{
    if (mType == "calendar") {
        mModel = Store::loadModel<Calendar>(query);
    } else if (mType == "addressbook") {
        mModel = Store::loadModel<Addressbook>(query);
    } else if (mType == "folder") {
        mModel = Store::loadModel<Folder>(query);
    } else if (mType == "todo") {
        mModel = Store::loadModel<Todo>(query);
    } else {
        qWarning() << "Type not supported " << mType;
        Q_ASSERT(false);
    }
    QObject::connect(mModel.data(), &QAbstractItemModel::dataChanged, this, [this](const QModelIndex &, const QModelIndex &, const QVector<int> &roles) {
        if (roles.contains(Sink::Store::ChildrenFetchedRole)) {
            emit initialItemsLoaded();
        }
    });
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
    if (!mResourceId.isEmpty()) {
        query.resourceFilter(mResourceId.toUtf8());
    }
    if (!mEntityId.isEmpty()) {
        query.filter(mEntityId.toUtf8());
    }
    query.setFlags(Sink::Query::LiveQuery | Sink::Query::UpdateStatus);

    for (const auto &property: mRoles.keys()) {
        query.requestedProperties << property;
    }

    for (const auto &property: mFilter.keys()) {
        query.filter(property.toUtf8(), {mFilter.value(property)});
    }

    runQuery(query);
}

void EntityModel::setAccountId(const QString &id)
{
    if (mAccountId != id) {
        mAccountId = id;
        updateQuery();
    }
}

QString EntityModel::accountId() const
{
    return mAccountId;
}

void EntityModel::setResourceId(const QString &id)
{
    if (mResourceId != id) {
        mResourceId = id;
        updateQuery();
    }
}

QString EntityModel::resourceId() const
{
    return mResourceId;
}

void EntityModel::setEntityId(const QString &id)
{
    if (mEntityId != id) {
        mEntityId = id;
        updateQuery();
    }
}

QString EntityModel::entityId() const
{
    return mEntityId;
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
    mRoleNames.insert(IdRole, "identifier");
    mRoleNames.insert(ObjectRole, "object");
    int role = LastRole;
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

void EntityModel::setFilter(const QVariantMap &filter)
{
    mFilter = filter;
    updateQuery();
}

QVariantMap EntityModel::filter() const
{
    return {};
}

void EntityModel::setSortRole(const QString &sortRole)
{
    mSortRole = sortRole;
    sort(0, Qt::AscendingOrder);
}

QString EntityModel::sortRole() const
{
    return mSortRole;
}


QVariantMap EntityModel::data(int row) const
{
    QVariantMap map;
    for (const auto &r : mRoleNames.keys()) {
        map.insert(mRoleNames.value(r), data(index(row, 0), r));
    }
    return map;
}

bool EntityModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!mRoleNames.contains(role)) {
        return false;
    }
    const auto entity = EntityModel::data(index, ObjectRole).value<Sink::ApplicationDomain::ApplicationDomainType::Ptr>();
    //FIXME hardcoding calendar is not a great idea here.
    Sink::ApplicationDomain::Calendar modifiedEntity{*entity};
    const auto propertyName = mRoleNames.value(role);
    modifiedEntity.setProperty(propertyName, value.toBool());
    //Ignore if we didn't modify anything.
    if (!modifiedEntity.changedProperties().isEmpty()) {
        Sink::Store::modify(modifiedEntity).exec();
    }
    return true;
}

EntityLoader::EntityLoader(QObject *parent) : EntityModel(parent)
{
    QObject::connect(this, &QAbstractItemModel::rowsInserted, this, [this] (const QModelIndex &parent, int first, int last) {
        for (int row = first; row <= last; row++) {
            auto idx = index(row, 0, parent);
            for (const auto &r : mRoleNames.keys()) {
                setProperty(mRoleNames.value(r), data(idx, r));
            }
            //We only ever use the first index (we're not expecting any more either)
            break;
        }
    });
}

EntityLoader::~EntityLoader()
{

}

void EntityLoader::updateQuery()
{
    if (entityId().isEmpty()) {
        for (const auto &r : mRoleNames.keys()) {
            setProperty(mRoleNames.value(r), {});
        }
        return;
    }
    EntityModel::updateQuery();
}


CheckableEntityModel::CheckableEntityModel(QObject *parent) : EntityModel(parent)
{
}

CheckableEntityModel::~CheckableEntityModel()
{

}

QHash<int, QByteArray > CheckableEntityModel::roleNames() const
{
    auto roleNames = EntityModel::roleNames();
    roleNames.insert(Qt::CheckStateRole, "checked");
    return roleNames;
}

QVariant CheckableEntityModel::data(const QModelIndex &index, int role) const
{
    if (mCheckedEntities && role == Qt::CheckStateRole) {
        return mCheckedEntities->contains(EntityModel::data(index, IdRole).toByteArray());
    }
    return EntityModel::data(index, role);
}

bool CheckableEntityModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (mCheckedEntities && role == Qt::CheckStateRole) {
        auto identifier = EntityModel::data(index, IdRole).toByteArray();
        if (value.toBool()) {
            mCheckedEntities->insert(identifier);
        } else {
            mCheckedEntities->remove(identifier);
        }
        return true;
    }
    return EntityModel::setData(index, value, role);
}

CheckedEntities *CheckableEntityModel::checkedEntities() const
{
    return mCheckedEntities;
}

void CheckableEntityModel::setCheckedEntities(CheckedEntities *checkedEntities)
{
    mCheckedEntities = checkedEntities;
}


void CheckedEntities::insert(const QByteArray &id)
{
    mCheckedEntities.insert(id);
    emit checkedEntitiesChanged();
}

void CheckedEntities::remove(const QByteArray &id)
{
    mCheckedEntities.remove(id);
    emit checkedEntitiesChanged();
}

bool CheckedEntities::contains(const QByteArray &id) const
{
    return mCheckedEntities.contains(id);
}

QSet<QByteArray> CheckedEntities::checkedEntities() const
{
    return mCheckedEntities;
}
