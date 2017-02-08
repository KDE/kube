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

#include <sink/standardqueries.h>

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
    roles[To] = "to";
    roles[Cc] = "cc";
    roles[Bcc] = "bcc";
    roles[Date] = "date";
    roles[Unread] = "unread";
    roles[Important] = "important";
    roles[Draft] = "draft";
    roles[Trash] = "trash";
    roles[Id] = "id";
    roles[MimeMessage] = "mimeMessage";
    roles[DomainObject] = "domainObject";
    roles[ThreadSize] = "threadSize";
    roles[Mail] = "mail";

    return roles;
}

static QString join(const QList<Sink::ApplicationDomain::Mail::Contact> &contacts)
{
    QStringList list;
    for (const auto &contact : contacts) {
        if (!contact.name.isEmpty()) {
            list << QString("%1 <%2>").arg(contact.name).arg(contact.emailAddress);
        } else {
            list << contact.emailAddress;
        }
    }
    return list.join(", ");
}

void fetchMail(Sink::ApplicationDomain::Mail::Ptr mail)
{
    if (mail && !mail->getFullPayloadAvailable()) {
        qWarning() << "Fetching mail: " << mail->identifier() << mail->getSubject();
        Sink::Store::synchronize(Sink::SyncScope{*mail}).exec();
    }
}

QVariant MailListModel::data(const QModelIndex &idx, int role) const
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
        case To:
            return join(mail->getTo());
        case Cc:
            return join(mail->getCc());
        case Bcc:
            return join(mail->getBcc());
        case Date:
            return mail->getDate();
        case Unread:
            return mail->getProperty("unreadCollected").toList().contains(true);
        case Important:
            return mail->getProperty("importantCollected").toList().contains(true);
        case Draft:
            return mail->getDraft();
        case Trash:
            return mail->getTrash();
        case Id:
            return mail->identifier();
        case DomainObject:
            return QVariant::fromValue(mail);
        case MimeMessage:
            if (mFetchMails) {
                fetchMail(mail);
            }
            return mail->getMimeMessage();
        case ThreadSize:
            return mail->getProperty("count").toInt();
        case Mail:
            return QVariant::fromValue(mail);
    }
    return QSortFilterProxyModel::data(idx, role);
}

bool MailListModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    const auto leftDate = left.data(Sink::Store::DomainObjectRole).value<Sink::ApplicationDomain::Mail::Ptr>()->getDate();
    const auto rightDate = right.data(Sink::Store::DomainObjectRole).value<Sink::ApplicationDomain::Mail::Ptr>()->getDate();
    return leftDate < rightDate;
}

void MailListModel::runQuery(const Sink::Query &query)
{
    m_model = Sink::Store::loadModel<Sink::ApplicationDomain::Mail>(query);
    setSourceModel(m_model.data());
}

void MailListModel::setParentFolder(const QVariant &parentFolder)
{
    using namespace Sink::ApplicationDomain;
    auto folder = parentFolder.value<Folder::Ptr>();
    if (!folder) {
        qWarning() << "No folder: " << parentFolder;
        setSourceModel(nullptr);
        return;
    }
    Sink::Query query = Sink::StandardQueries::threadLeaders(*folder);
    query.setFlags(Sink::Query::LiveQuery);
    query.limit(100);
    query.request<Mail::Subject>();
    query.request<Mail::Sender>();
    query.request<Mail::To>();
    query.request<Mail::Cc>();
    query.request<Mail::Bcc>();
    query.request<Mail::Date>();
    query.request<Mail::Unread>();
    query.request<Mail::Important>();
    query.request<Mail::Draft>();
    query.request<Mail::Trash>();
    query.request<Mail::Folder>();
    mFetchMails = false;
    qWarning() << "Running folder query: " << folder->resourceInstanceIdentifier() << folder->identifier();
    //Latest mail on top
    sort(0, Qt::DescendingOrder);
    runQuery(query);
}

QVariant MailListModel::parentFolder() const
{
    return QVariant();
}

void MailListModel::setMail(const QVariant &variant)
{
    using namespace Sink::ApplicationDomain;
    auto mail = variant.value<Sink::ApplicationDomain::Mail::Ptr>();
    if (!mail) {
        qWarning() << "No mail: " << mail;
        setSourceModel(nullptr);
        return;
    }
    Sink::Query query = Sink::StandardQueries::completeThread(*mail);
    query.request<Mail::Subject>();
    query.request<Mail::Sender>();
    query.request<Mail::To>();
    query.request<Mail::Cc>();
    query.request<Mail::Bcc>();
    query.request<Mail::Date>();
    query.request<Mail::Unread>();
    query.request<Mail::Important>();
    query.request<Mail::Draft>();
    query.request<Mail::Trash>();
    query.request<Mail::MimeMessage>();
    query.request<Mail::FullPayloadAvailable>();
    mFetchMails = true;
    qWarning() << "Running mail query: " << mail->resourceInstanceIdentifier() << mail->identifier();
    //Latest mail at the bottom
    sort(0, Qt::AscendingOrder);
    runQuery(query);
}

QVariant MailListModel::mail() const
{
    return QVariant();
}


