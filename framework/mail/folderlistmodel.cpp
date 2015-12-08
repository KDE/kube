#include "folderlistmodel.h"
#include <akonadi2common/clientapi.h>

FolderListModel::FolderListModel(QObject *parent) : QIdentityProxyModel()
{
    Akonadi2::Query query;
    query.syncOnDemand = false;
    query.processAll = false;
    query.liveQuery = true;
    query.requestedProperties << "name" << "icon";
    mModel = Akonadi2::Store::loadModel<Akonadi2::ApplicationDomain::Folder>(query);
    setSourceModel(mModel.data());
}

FolderListModel::~FolderListModel()
{

}

QHash< int, QByteArray > FolderListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[Name] = "name";
    roles[Icon] = "icon";

    return roles;
}

QVariant FolderListModel::data(const QModelIndex &idx, int role) const
{
    switch (role) {
        case Name:
            return mapToSource(idx).data(Qt::DisplayRole).toString();
	case Icon:
	    return mapToSource(idx).data(Qt::DisplayRole).toString();
    }
    return QIdentityProxyModel::data(idx, role);
}
