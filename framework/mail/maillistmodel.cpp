#include "maillistmodel.h"
#include <akonadi2common/clientapi.h>


MailListModel::MailListModel(QObject *parent)
    : QIdentityProxyModel()
{
    Akonadi2::Query query;
    query.syncOnDemand = false;
    query.processAll = false;
    query.liveQuery = true;
    query.requestedProperties << "subject" << "sender" << "senderName" << "date" << "unread" << "important";
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
    roles[Sender] = "sender";
    roles[SenderName] = "senderName";
    roles[Date] = "date";
    roles[Unread] = "unread";
    roles[Important] = "important";

    return roles;
}

QVariant MailListModel::data(const QModelIndex &idx, int role) const
{
    auto srcIdx = mapToSource(idx);
    switch (role) {
        case Subject:
            return srcIdx.sibling(srcIdx.row(), 0).data(Qt::DisplayRole).toString();
        case Sender:
            return srcIdx.sibling(srcIdx.row(), 1).data(Qt::DisplayRole).toString();
        case SenderName:
            return srcIdx.sibling(srcIdx.row(), 2).data(Qt::DisplayRole).toString();
        case Date:
            return srcIdx.sibling(srcIdx.row(), 3).data(Qt::DisplayRole).toString();
        case Unread:
            return srcIdx.sibling(srcIdx.row(), 4).data(Qt::DisplayRole).toString();
        case Important:
            return srcIdx.sibling(srcIdx.row(), 5).data(Qt::DisplayRole).toString();
    }
    return QIdentityProxyModel::data(idx, role);
}

void MailListModel::runQuery(const QString& query)
{
}

