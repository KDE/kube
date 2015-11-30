#include "maillistmodel.h"
#include <akonadi2common/clientapi.h>


MailListModel::MailListModel(QObject *parent)
    : QIdentityProxyModel()
{
    Akonadi2::Query query;
    query.syncOnDemand = false;
    query.processAll = false;
    query.liveQuery = true;
    QList<QByteArray> requestedProperties;
    requestedProperties << "subject";
    query.requestedProperties = requestedProperties.toSet();
    mModel = Akonadi2::Store::loadModel<Akonadi2::ApplicationDomain::Mail>(query);
    setSourceModel(mModel.data());
}

MailListModel::~MailListModel()
{

}

QHash< int, QByteArray > MailListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[Subject] = "subject";

    return roles;
}

QVariant MailListModel::data(const QModelIndex &idx, int role) const
{
    switch (role) {
        case Subject:
            return mapToSource(idx).data(Qt::DisplayRole).toString();
    }
    return QIdentityProxyModel::data(idx, role);
}

void MailListModel::runQuery(const QString& query)
{
}

