#pragma once

#include <QAbstractListModel>
#include <QStringList>

class MailListModel : public QAbstractListModel
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
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    bool addMails(const QStringList &items);
    void clearMails();

    void runQuery(const QString &query);

private:
    QStringList m_msgs;
};