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
#include <sink/query.h>
#include <sink/store.h>
#include <QSharedPointer>
#include <QAbstractItemModel>
#include <QByteArrayList>

class KUBE_EXPORT EntityCacheInterface
{
public:
    typedef QSharedPointer<EntityCacheInterface> Ptr;
    EntityCacheInterface() = default;
    virtual ~EntityCacheInterface() = default;

    virtual QVariant getProperty(const QByteArray &identifier, const QByteArray &property) const = 0;
};

template<typename DomainType>
class KUBE_EXPORT EntityCache : public EntityCacheInterface
{
public:
    typedef QSharedPointer<EntityCache> Ptr;

    EntityCache(const QByteArrayList &properties);
    virtual ~EntityCache() = default;

    virtual QVariant getProperty(const QByteArray &, const QByteArray &) const override;

private:
    QHash<QByteArray, typename DomainType::Ptr> mCache;
    QSharedPointer<QAbstractItemModel> mModel;
};

template<typename DomainType>
EntityCache<DomainType>::EntityCache(const QByteArrayList &properties)
    : EntityCacheInterface()
{
    Sink::Query query;
    query.requestedProperties = properties;
    query.setFlags(Sink::Query::LiveQuery);
    mModel = Sink::Store::loadModel<DomainType>(query);
    QObject::connect(mModel.data(), &QAbstractItemModel::rowsInserted, mModel.data(), [this] (const QModelIndex &, int start, int end) {
        for (int row = start; row <= end; row++) {
            auto entity = mModel->index(row, 0, QModelIndex()).data(Sink::Store::DomainObjectRole).template value<typename DomainType::Ptr>();
            mCache.insert(entity->identifier(), entity);
        }
    });
}

template<typename DomainType>
QVariant EntityCache<DomainType>::getProperty(const QByteArray &identifier, const QByteArray &property) const
{
    if (auto entity = mCache.value(identifier)) {
        return entity->getProperty(property);
    }
    return {};
}

