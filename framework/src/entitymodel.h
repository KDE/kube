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
#include <QSharedPointer>
#include <QSortFilterProxyModel>
#include <QSet>
#include <QByteArray>

namespace Sink {
    class Query;
}

class KUBE_EXPORT EntityModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY (QString accountId READ accountId WRITE setAccountId)
    Q_PROPERTY (QString resourceId READ resourceId WRITE setResourceId)
    Q_PROPERTY (QString entityId READ entityId WRITE setEntityId)
    Q_PROPERTY (QString type READ type WRITE setType)
    Q_PROPERTY (QStringList roles READ roles WRITE setRoles)
    Q_PROPERTY (QString sortRole READ sortRole WRITE setSortRole)
    Q_PROPERTY (QVariantMap filter READ filter WRITE setFilter)

public:
    enum Status {
        NoStatus,
        InProgressStatus,
        ErrorStatus,
        SuccessStatus,
    };
    Q_ENUMS(Status)
    EntityModel(QObject *parent = Q_NULLPTR);
    virtual ~EntityModel();

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    virtual QHash<int, QByteArray> roleNames() const override;

    void setAccountId(const QString &);
    QString accountId() const;

    void setResourceId(const QString &);
    QString resourceId() const;

    void setEntityId(const QString &);
    QString entityId() const;

    void setType(const QString &);
    QString type() const;

    void setRoles(const QStringList &);
    QStringList roles() const;

    void setFilter(const QVariantMap &);
    QVariantMap filter() const;

    void setSortRole(const QString &);
    QString sortRole() const;

    Q_INVOKABLE QVariantMap data(int row) const;

protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

    virtual void updateQuery();
    QHash<int, QByteArray> mRoleNames;

private:
    void runQuery(const Sink::Query &query);
    QSharedPointer<QAbstractItemModel> mModel;
    QHash<QByteArray, int> mRoles;
    QString mAccountId;
    QString mResourceId;
    QString mEntityId;
    QString mType;
    QString mSortRole;
};

class KUBE_EXPORT EntityLoader : public EntityModel {
    Q_OBJECT
public:
    EntityLoader(QObject *parent = Q_NULLPTR);
    virtual ~EntityLoader();
protected:
    void updateQuery() override;
};

class KUBE_EXPORT CheckedEntities : public QObject {
    Q_OBJECT
    Q_PROPERTY (QSet<QByteArray> checkedEntities READ checkedEntities NOTIFY checkedEntitiesChanged)
public:
    bool contains(const QByteArray &) const;
    void insert(const QByteArray &);
    void remove(const QByteArray &);
    QSet<QByteArray> checkedEntities() const;

signals:
    void checkedEntitiesChanged();

private:
    QSet<QByteArray> mCheckedEntities;
};

class KUBE_EXPORT CheckableEntityModel : public EntityModel {

    Q_OBJECT
    Q_PROPERTY (CheckedEntities* checkedEntities READ checkedEntities WRITE setCheckedEntities CONSTANT)
public:
    CheckableEntityModel(QObject *parent = Q_NULLPTR);
    virtual ~CheckableEntityModel();

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    CheckedEntities *checkedEntities() const;
    void setCheckedEntities(CheckedEntities *);

private:
    CheckedEntities *mCheckedEntities = nullptr;
};
