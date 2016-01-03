#pragma once

#include <akonadi2common/clientapi.h>

#include <QIdentityProxyModel>
#include <QSharedPointer>
#include <QStringList>

class MailListModel : public QIdentityProxyModel
{
    Q_OBJECT
    Q_PROPERTY (QVariant parentFolder READ parentFolder WRITE setParentFolder)
    Q_PROPERTY (QVariant mail READ mail WRITE setMail)

public:
    MailListModel(QObject *parent = Q_NULLPTR);
    ~MailListModel();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    enum Roles {
        Subject  = Qt::UserRole + 1,
        Sender,
        SenderName,
        Date,
        Unread,
        Important,
        Id,
        MimeMessage,
        DomainObject
    };

    QHash<int, QByteArray> roleNames() const;

    void runQuery(const Akonadi2::Query &query);

    void setParentFolder(const QVariant &parentFolder);
    QVariant parentFolder() const;

    void setMail(const QVariant &mail);
    QVariant mail() const;
private:
    QSharedPointer<QAbstractItemModel> m_model;
};
