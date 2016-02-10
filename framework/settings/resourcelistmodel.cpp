#include "resourcelistmodel.h"

#include <sink/store.h>

ResourceListModel::ResourceListModel(QObject *parent) : QIdentityProxyModel()
{
    Sink::Query query;
    query.syncOnDemand = false;
    query.processAll = false;
    query.liveQuery = true;
    query.requestedProperties << "type";
    m_model = Sink::Store::loadModel<Sink::ApplicationDomain::SinkResource>(query);
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
            return srcIdx.data(Sink::Store::DomainObjectBaseRole).value<Sink::ApplicationDomain::ApplicationDomainType::Ptr>()->identifier();
        case Type:
            return srcIdx.sibling(srcIdx.row(), 0).data(Qt::DisplayRole).toString();
    }

    return QIdentityProxyModel::data(index, role);
}

