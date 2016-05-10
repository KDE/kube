/*
    Copyright (c) 2016 Michael Bohlender <michael.bohlender@kdemail.net>
    Copyright (c) 2016 Christian Mollekopf <mollekopf@kolabsys.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "maillistmodel.h"

#include <QFile>
#include <QDateTime>


MailListModel::MailListModel(QObject *parent)
    : QSortFilterProxyModel()
{
    setDynamicSortFilter(true);
    sort(0, Qt::DescendingOrder);
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
    auto mail = srcIdx.data(Sink::Store::DomainObjectRole).value<Sink::ApplicationDomain::Mail::Ptr>();
    switch (role) {
        case Subject:
            return mail->getSubject();
        case Sender:
            return mail->getSender();
        case SenderName:
            return mail->getSenderName();
        case Date:
            return mail->getDate();
        case Unread:
            return mail->getUnread();
        case Important:
            return mail->getImportant();
        case Id:
            return mail->identifier();
        case DomainObject:
            return QVariant::fromValue(mail);
        case MimeMessage: {
            return mail->getMimeMessage();
        }
    }
    return QSortFilterProxyModel::data(idx, role);
}

bool MailListModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    const QVariant leftData = left.sibling(left.row(), 3).data(Qt::DisplayRole);
    const QVariant rightData = right.sibling(right.row(), 3).data(Qt::DisplayRole);
    return leftData.toDateTime() < rightData.toDateTime();
}

void MailListModel::runQuery(const Sink::Query &query)
{
    m_model = Sink::Store::loadModel<Sink::ApplicationDomain::Mail>(query);
    setSourceModel(m_model.data());
}

void MailListModel::setParentFolder(const QVariant &parentFolder)
{
    auto folder = parentFolder.value<Sink::ApplicationDomain::Folder::Ptr>();
    if (!folder) {
        qWarning() << "No folder: " << parentFolder;
        return;
    }
    Sink::Query query;
    query.liveQuery = true;
    query.requestedProperties << "subject" << "sender" << "senderName" << "date" << "unread" << "important" << "folder";
    query.resources << folder->resourceInstanceIdentifier();
    query.sortProperty = "date";
    query.limit = 100;
    query += Sink::Query::PropertyFilter("folder", *folder);
    qWarning() << "Running folder query: " << folder->resourceInstanceIdentifier() << folder->identifier();
    runQuery(query);
}

QVariant MailListModel::parentFolder() const
{
    return QVariant();
}

void MailListModel::setMail(const QVariant &variant)
{
    auto mail = variant.value<Sink::ApplicationDomain::Mail::Ptr>();
    if (!mail) {
        qWarning() << "No mail: " << mail;
        return;
    }
    Sink::Query query;
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


