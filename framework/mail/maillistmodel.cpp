#include "maillistmodel.h"

MailListModel::MailListModel(QObject *parent) : ListModelResult<Akonadi2::ApplicationDomain::Mail::Ptr>(QList<QByteArray>() << "subject" << "uid")
{
    Akonadi2::Query query;
    query.syncOnDemand = false;
    query.processAll = false;
    query.liveQuery = true;
    setEmitter(Akonadi2::Store::load<Akonadi2::ApplicationDomain::Mail>(query));
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
            return ListModelResult<Akonadi2::ApplicationDomain::Mail::Ptr>::data(index(idx.row(), 0, idx.parent()), Qt::DisplayRole);
    }
    return QVariant();
}

void MailListModel::runQuery(const QString& query)
{
}

