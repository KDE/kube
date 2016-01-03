#include "maillistmodel.h"

#include <QFile>

MailListModel::MailListModel(QObject *parent)
    : QIdentityProxyModel()
{

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
    roles[Id] = "id";
    roles[MimeMessage] = "mimeMessage";
    roles[DomainObject] = "domainObject";

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
        case Id:
            return srcIdx.data(Akonadi2::Store::DomainObjectBaseRole).value<Akonadi2::ApplicationDomain::ApplicationDomainType::Ptr>()->identifier();
        case DomainObject:
            return srcIdx.data(Akonadi2::Store::DomainObjectRole);
        case MimeMessage: {
            auto filename = srcIdx.sibling(srcIdx.row(), 6).data(Qt::DisplayRole).toString();
            QFile file(filename);
            if (file.open(QFile::ReadOnly)) {
                auto content = file.readAll();
                return content;
            } else {
                qWarning() << "Failed to open the file";
            }
            return "Failed to read mail.";
        }
    }
    return QIdentityProxyModel::data(idx, role);
}

void MailListModel::runQuery(const Akonadi2::Query &query)
{
    m_model = Akonadi2::Store::loadModel<Akonadi2::ApplicationDomain::Mail>(query);
    setSourceModel(m_model.data());
}

void MailListModel::setParentFolder(const QVariant &parentFolder)
{
    auto folder = parentFolder.value<Akonadi2::ApplicationDomain::Folder::Ptr>();
    if (!folder) {
        qWarning() << "No folder: " << parentFolder;
        return;
    }
    Akonadi2::Query query;
    query.syncOnDemand = false;
    query.processAll = false;
    query.liveQuery = true;
    query.requestedProperties << "subject" << "sender" << "senderName" << "date" << "unread" << "important" << "folder";
    query.propertyFilter.insert("folder", folder->identifier());
    query.resources << folder->resourceInstanceIdentifier();
    qWarning() << "Running folder query: " << folder->resourceInstanceIdentifier() << folder->identifier();
    runQuery(query);
}

QVariant MailListModel::parentFolder() const
{
    return QVariant();
}

void MailListModel::setMail(const QVariant &variant)
{
    auto mail = variant.value<Akonadi2::ApplicationDomain::Mail::Ptr>();
    if (!mail) {
        qWarning() << "No mail: " << mail;
        return;
    }
    Akonadi2::Query query;
    query.syncOnDemand = false;
    query.processAll = false;
    query.liveQuery = false;
    query.requestedProperties << "subject" << "sender" << "senderName" << "date" << "unread" << "important" << "mimeMessage";
    query.ids << mail->identifier();
    query.resources << mail->resourceInstanceIdentifier();
    qWarning() << "Running mail query: " << mail->resourceInstanceIdentifier() << mail->identifier();
    runQuery(query);
}

QVariant MailListModel::mail() const
{
    return QVariant();
}


