#include "maillistmodel.h"

#include <QDateTime>

MailListModel::MailListModel(QObject *parent) : QAbstractListModel(parent), m_msgs()
{

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


QVariant MailListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= m_msgs.count() || index.row() < 0) {
        return QVariant();
    }
        switch (role) {
        case Subject:
            return m_msgs.at(index.row());
        }
    return QVariant();
}

int MailListModel::rowCount(const QModelIndex &parent) const
{
    return m_msgs.size();
}

bool MailListModel::addMails(const QStringList &items)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount() + items.size() - 1);

    m_msgs += items;

    endInsertRows();

    return true;
}

void MailListModel::clearMails()
{
    if (!m_msgs.isEmpty()) {
        beginResetModel();
        m_msgs.clear();
        endResetModel();
    }
}

void MailListModel::runQuery(const QString& query)
{
    clearMails();
    QStringList itemlist;
    itemlist << "I feel tiny" << "Big News!" <<  "[FUN] lets do things";
    addMails(itemlist);
}

