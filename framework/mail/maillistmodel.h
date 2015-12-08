#pragma once

#include <akonadi2common/clientapi.h>

#include <QIdentityProxyModel>
#include <QSharedPointer>
#include <QStringList>

class MailListModel : public QIdentityProxyModel
{
    Q_OBJECT

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
        Important
    };

    QHash<int, QByteArray> roleNames() const;

    void runQuery(const Akonadi2::Query &query);
private:
    QSharedPointer<QAbstractItemModel> m_model;
};
