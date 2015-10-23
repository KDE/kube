#pragma once

#include <QAbstractListModel>
#include <QStringList>

#include <akonadi2common/clientapi.h>
#include <akonadi2common/query.h>
#include <akonadi2common/listmodelresult.h>

class MailListModel : public ListModelResult<Akonadi2::ApplicationDomain::Mail::Ptr>
{
    Q_OBJECT

public:
    MailListModel(QObject *parent = Q_NULLPTR);
    ~MailListModel();

    enum Roles {
        Subject  = Qt::UserRole + 1
    };

    QHash<int, QByteArray> roleNames() const;
    QVariant data(const QModelIndex &index, int role) const;

    void runQuery(const QString &query);
};
