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

#include "outboxmodel.h"

#include <QFile>
#include <QDateTime>

#include <sink/standardqueries.h>


OutboxModel::OutboxModel(QObject *parent)
    : QSortFilterProxyModel()
{
    setDynamicSortFilter(true);
    sort(0, Qt::DescendingOrder);

    using namespace Sink::ApplicationDomain;
    auto query = Sink::StandardQueries::outboxMails();
    query.setFlags(Sink::Query::LiveQuery);
    query.request<Mail::Subject>();
    query.request<Mail::Sender>();
    query.request<Mail::Date>();
    query.request<Mail::Unread>();
    query.request<Mail::Important>();
    query.request<Mail::Draft>();
    query.request<Mail::Folder>();
    runQuery(query);
}

OutboxModel::~OutboxModel()
{

}

QHash< int, QByteArray > OutboxModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[Subject] = "subject";
    roles[Sender] = "sender";
    roles[SenderName] = "senderName";
    roles[Date] = "date";
    roles[Unread] = "unread";
    roles[Important] = "important";
    roles[Draft] = "draft";
    roles[Id] = "id";
    roles[MimeMessage] = "mimeMessage";
    roles[DomainObject] = "domainObject";

    return roles;
}

QVariant OutboxModel::data(const QModelIndex &idx, int role) const
{
    auto srcIdx = mapToSource(idx);
    auto mail = srcIdx.data(Sink::Store::DomainObjectRole).value<Sink::ApplicationDomain::Mail::Ptr>();
    switch (role) {
        case Subject:
            return mail->getSubject();
        case Sender:
            return mail->getSender().emailAddress;
        case SenderName:
            return mail->getSender().name;
        case Date:
            return mail->getDate();
        case Unread:
            return mail->getUnread();
        case Important:
            return mail->getImportant();
        case Draft:
            return mail->getDraft();
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

bool OutboxModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    const auto leftDate = left.data(Sink::Store::DomainObjectRole).value<Sink::ApplicationDomain::Mail::Ptr>()->getDate();
    const auto rightDate = right.data(Sink::Store::DomainObjectRole).value<Sink::ApplicationDomain::Mail::Ptr>()->getDate();
    return leftDate < rightDate;
}

void OutboxModel::runQuery(const Sink::Query &query)
{
    m_model = Sink::Store::loadModel<Sink::ApplicationDomain::Mail>(query);
    setSourceModel(m_model.data());
}

