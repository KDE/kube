#pragma once

#include <QObject>
#include <QIdentityProxyModel>
#include <QSharedPointer>
#include <QStringList>

class FolderListModel : public QIdentityProxyModel
{
    Q_OBJECT

public:
    FolderListModel(QObject *parent = Q_NULLPTR);
    ~FolderListModel();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    enum Roles {
        Name  = Qt::UserRole + 1,
        Icon,
        Id,
        DomainObject
    };

    QHash<int, QByteArray> roleNames() const;

private:
    QSharedPointer<QAbstractItemModel> mModel;
};
