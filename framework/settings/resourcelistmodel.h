#pragma once

#include <QIdentityProxyModel>
#include <QSharedPointer>
#include <QStringList>

class ResourceListModel : public QIdentityProxyModel
{
    Q_OBJECT

public:
    ResourceListModel(QObject *parent = Q_NULLPTR);
    ~ResourceListModel();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    enum Roles {
        Id  = Qt::UserRole + 1,
        Type
    };

    QHash<int, QByteArray> roleNames() const;

private:
    QSharedPointer<QAbstractItemModel> m_model;
};
