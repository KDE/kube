/*
    Copyright (c) 2016 Michael Bohlender <michael.bohlender@kdemail.net>
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

#include "folderlistmodel.h"
#include <sink/store.h>
#include <sink/log.h>
#include <settings/settings.h>

using namespace Sink;
using namespace Sink::ApplicationDomain;

FolderListModel::FolderListModel(QObject *parent) : KRecursiveFilterProxyModel()
{
    setDynamicSortFilter(true);
    sort(0, Qt::AscendingOrder);

    //Automatically fetch all folders, otherwise the recursive filtering does not work.
    QObject::connect(this, &QSortFilterProxyModel::sourceModelChanged, [this] () {
        if (sourceModel()) {
            QObject::connect(sourceModel(), &QAbstractItemModel::rowsInserted, sourceModel(), [this] (QModelIndex parent, int first, int last) {
                for (int row = first; row <= last; row++) {
                    auto idx = sourceModel()->index(row, 0, parent);
                    sourceModel()->fetchMore(idx);
                }
            });
        }
    });
}

FolderListModel::~FolderListModel()
{

}

QHash< int, QByteArray > FolderListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[Name] = "name";
    roles[Icon] = "icon";
    roles[Id] = "id";
    roles[DomainObject] = "domainObject";
    roles[Status] = "status";
    roles[Trash] = "trash";

    return roles;
}

QVariant FolderListModel::data(const QModelIndex &idx, int role) const
{
    auto srcIdx = mapToSource(idx);
    auto folder = srcIdx.data(Sink::Store::DomainObjectRole).value<Sink::ApplicationDomain::Folder::Ptr>();
    switch (role) {
        case Name:
            return folder->getName();
        case Icon:
            return folder->getIcon();
        case Id:
            return folder->identifier();
        case DomainObject:
            return QVariant::fromValue(folder);
        case Status: {
            switch (srcIdx.data(Sink::Store::StatusRole).toInt()) {
                case Sink::ApplicationDomain::SyncStatus::SyncInProgress:
                    return InProgressStatus;
                case Sink::ApplicationDomain::SyncStatus::SyncError:
                    return ErrorStatus;
                case Sink::ApplicationDomain::SyncStatus::SyncSuccess:
                    return SuccessStatus;
            }
            return NoStatus;
        }
        case Trash:
            if (folder) {
                return folder->getSpecialPurpose().contains(Sink::ApplicationDomain::SpecialPurpose::Mail::trash);
            }
            return false;
    }
    return QSortFilterProxyModel::data(idx, role);
}

void FolderListModel::runQuery(const Query &query)
{
    mModel = Store::loadModel<Folder>(query);
    setSourceModel(mModel.data());
}

void FolderListModel::setAccountId(const QVariant &accountId)
{
    const auto account = accountId.toString().toUtf8();

    //Get all folders of an account
    auto query = Query();
    query.resourceFilter<SinkResource::Account>(account);
    query.setFlags(Sink::Query::LiveQuery | Sink::Query::UpdateStatus);
    query.request<Folder::Name>()
         .request<Folder::Icon>()
         .request<Folder::Parent>()
         .request<Folder::Enabled>()
         .request<Folder::SpecialPurpose>();
    query.requestTree<Folder::Parent>();
    query.setId("foldertree" + account);
    runQuery(query);
}

QVariant FolderListModel::accountId() const
{
    return {};
}

static int getPriority(const Sink::ApplicationDomain::Folder &folder)
{
    auto specialPurpose = folder.getSpecialPurpose();
    if (specialPurpose.contains(Sink::ApplicationDomain::SpecialPurpose::Mail::inbox)) {
        return 5;
    } else if (specialPurpose.contains(Sink::ApplicationDomain::SpecialPurpose::Mail::drafts)) {
        return 6;
    } else if (specialPurpose.contains(Sink::ApplicationDomain::SpecialPurpose::Mail::sent)) {
        return 7;
    } else if (specialPurpose.contains(Sink::ApplicationDomain::SpecialPurpose::Mail::trash)) {
        return 8;
    } else if (!specialPurpose.isEmpty()) {
        return 9;
    }
    return 10;
}

bool FolderListModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    const auto leftFolder = left.data(Sink::Store::DomainObjectRole).value<Sink::ApplicationDomain::Folder::Ptr>();
    const auto rightFolder = right.data(Sink::Store::DomainObjectRole).value<Sink::ApplicationDomain::Folder::Ptr>();
    const auto leftPriority = getPriority(*leftFolder);
    const auto rightPriority = getPriority(*rightFolder);
    if (leftPriority == rightPriority) {
        return leftFolder->getName() < rightFolder->getName();
    }
    return leftPriority < rightPriority;
}

bool FolderListModel::acceptRow(int sourceRow, const QModelIndex &sourceParent) const
{
    auto index = sourceModel()->index(sourceRow, 0, sourceParent);
    Q_ASSERT(index.isValid());
    const auto folder = index.data(Sink::Store::DomainObjectRole).value<Sink::ApplicationDomain::Folder::Ptr>();
    Q_ASSERT(folder);
    const auto enabled = folder->getEnabled();
    return enabled;
}

void FolderListModel::setFolderId(const QVariant &folderId)
{
    const auto folder = folderId.toString().toUtf8();
    if (folder.isEmpty()) {
        setSourceModel(nullptr);
        mModel.clear();
        return;
    }

    //Get all folders of an account
    auto query = Query();
    query.filter(folder);
    query.request<Folder::Name>()
         .request<Folder::Icon>()
         .request<Folder::Parent>()
         .request<Folder::SpecialPurpose>();
    query.setId("folder" + folder);
    runQuery(query);
}

QVariant FolderListModel::folderId() const
{
    return {};
}
