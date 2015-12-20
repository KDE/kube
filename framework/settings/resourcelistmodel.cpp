#include "resourcelistmodel.h"

#include <akonadi2common/clientapi.h>

ResourceListModel::ResourceListModel(QObject *parent) : QIdentityProxyModel()
{
    Akonadi2::Query query;
    query.syncOnDemand = false;
    query.processAll = false;
    query.liveQuery = true;
    query.requestedProperties << "type";
    m_model = Akonadi2::Store::loadModel<Akonadi2::ApplicationDomain::AkonadiResource>(query);
}

ResourceListModel::~ResourceListModel()
{

}

QHash< int, QByteArray > ResourceListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[Type] = "type";
    roles[Id] = "id";

    return roles;
}

QVariant ResourceListModel::data(const QModelIndex& index, int role) const
{
    auto srcIdx = mapToSource(index);
    switch (role) {
        case Id:
            return srcIdx.data(Akonadi2::Store::DomainObjectBaseRole).value<Akonadi2::ApplicationDomain::ApplicationDomainType::Ptr>()->identifier();
        case Type:
            return srcIdx.sibling(srcIdx.row(), 0).data(Qt::DisplayRole).toString();
    }

    return QIdentityProxyModel::data(index, role);
}

