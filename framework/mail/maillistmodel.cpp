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
    switch (role) {
        case Subject:
            return mapToSource(idx).data(Qt::DisplayRole).toString();
	case Sender:
	    return mapToSource(idx).data(Qt::DisplayRole).toString();
	case SenderName:
	    return mapToSource(idx).data(Qt::DisplayRole).toString();
	case Date:
	    return mapToSource(idx).data(Qt::DisplayRole).toString();
	case Unread:
	    return mapToSource(idx).data(Qt::DisplayRole).toString();
	case Important:
	    return mapToSource(idx).data(Qt::DisplayRole).toString();
    }
    return QIdentityProxyModel::data(idx, role);
}

void MailListModel::runQuery(const QString& query)
{
}

