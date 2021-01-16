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

#include <sink/standardqueries.h>

MailListModel::MailListModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    sort(0, Qt::DescendingOrder);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

MailListModel::~MailListModel()
{

}


static void requestHeaders(Sink::Query &query)
{
    using namespace Sink::ApplicationDomain;
    query.request<Mail::Subject>();
    query.request<Mail::Sender>();
    query.request<Mail::To>();
    query.request<Mail::Cc>();
    query.request<Mail::Bcc>();
    query.request<Mail::Date>();
    query.request<Mail::Unread>();
    query.request<Mail::Important>();
    query.request<Mail::Draft>();
    query.request<Mail::Folder>();
    query.request<Mail::Sent>();
    query.request<Mail::Trash>();
}

static void requestFullMail(Sink::Query &query)
{
    using namespace Sink::ApplicationDomain;
    requestHeaders(query);
    query.request<Mail::MimeMessage>();
    query.request<Mail::FullPayloadAvailable>();
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
    roles[Sent] = "sent";
    roles[Trash] = "trash";
    roles[Id] = "id";
    roles[MimeMessage] = "mimeMessage";
    roles[DomainObject] = "domainObject";
    roles[ThreadSize] = "threadSize";
    roles[Mail] = "mail";
    roles[Incomplete] = "incomplete";
    roles[Status] = "status";

    return roles;
}

static QString join(const QList<Sink::ApplicationDomain::Mail::Contact> &contacts)
{
    QStringList list;
    for (const auto &contact : contacts) {
        if (!contact.name.isEmpty()) {
            if (contact.name.contains(",")) {
                list << QString("\"%1\" <%2>").arg(contact.name).arg(contact.emailAddress);
            } else {
                list << QString("%1 <%2>").arg(contact.name).arg(contact.emailAddress);
            }
        } else {
            list << contact.emailAddress;
        }
    }
    return list.join(", ");
}

void MailListModel::fetchMail(Sink::ApplicationDomain::Mail::Ptr mail)
{
    if (mail && !mail->getFullPayloadAvailable() && !mFetchedMails.contains(mail->identifier())) {
        qDebug() << "Fetching mail: " << mail->identifier() << mail->getSubject();
        mFetchedMails.insert(mail->identifier());
        Sink::Store::synchronize(Sink::SyncScope{*mail}).exec();
    }
}

QVariant MailListModel::data(const QModelIndex &idx, int role) const
{
    auto srcIdx = mapToSource(idx);
    auto mail = srcIdx.data(Sink::Store::DomainObjectRole).value<Sink::ApplicationDomain::Mail::Ptr>();
    switch (role) {
        case Subject:
            if (mail->isAggregate()) {
                return mail->getProperty(QByteArray{Sink::ApplicationDomain::Mail::Subject::name} + QByteArray{"Selected"});
            } else {
                return mail->getSubject();
            }
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
            if (mail->isAggregate()) {
                return mail->getCollectedProperty<Sink::ApplicationDomain::Mail::Unread>().contains(true);
            } else {
                return mail->getUnread();
            }
        case Important:
            if (mail->isAggregate()) {
                return mail->getCollectedProperty<Sink::ApplicationDomain::Mail::Important>().contains(true);
            } else {
                return mail->getImportant();
            }
        case Draft:
            return mail->getDraft();
        case Sent:
            return mail->getSent();
        case Trash:
            return mail->getTrash();
        case Id:
            return mail->identifier();
        case DomainObject:
            return QVariant::fromValue(mail);
        case MimeMessage:
            if (mFetchMails) {
                const_cast<MailListModel*>(this)->fetchMail(mail);
            }
            return mail->getMimeMessage();
        case ThreadSize:
            return mail->count();
        case Mail:
            return QVariant::fromValue(mail);
        case Incomplete:
            return !mail->getFullPayloadAvailable();
        case Status:
            const auto status = srcIdx.data(Sink::Store::StatusRole).toInt();
            if (status == Sink::ApplicationDomain::SyncStatus::SyncInProgress) {
                return InProgressStatus;
            }
            if (status == Sink::ApplicationDomain::SyncStatus::SyncError) {
                return ErrorStatus;
            }
            return NoStatus;
    }
    return QSortFilterProxyModel::data(idx, role);
}

bool MailListModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    const auto leftDate = left.data(Sink::Store::DomainObjectRole).value<Sink::ApplicationDomain::Mail::Ptr>()->getDate();
    const auto rightDate = right.data(Sink::Store::DomainObjectRole).value<Sink::ApplicationDomain::Mail::Ptr>()->getDate();
    if (leftDate == rightDate) {
        return left.data(Sink::Store::DomainObjectRole).value<Sink::ApplicationDomain::Mail::Ptr>()->identifier() <
                right.data(Sink::Store::DomainObjectRole).value<Sink::ApplicationDomain::Mail::Ptr>()->identifier();
    }
    return leftDate < rightDate;
}

bool MailListModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    auto idx = sourceModel()->index(sourceRow, 0, sourceParent);
    auto regExp = filterRegExp();
    if (regExp.isEmpty()) {
        return true;
    }
    auto mail = idx.data(Sink::Store::DomainObjectRole).value<Sink::ApplicationDomain::Mail::Ptr>();
    return mail->getSubject().contains(regExp) ||
        mail->getSender().name.contains(regExp);
}

void MailListModel::runQuery(const Sink::Query &query)
{
    if (mQuery == query) {
        return;
    }
    if (query.getBaseFilters().isEmpty() && query.ids().isEmpty()) {
        mQuery = {};
        m_model.clear();
        setSourceModel(nullptr);
    } else {
        mQuery = query;
        m_model = Sink::Store::loadModel<Sink::ApplicationDomain::Mail>(query);
        setSourceModel(m_model.data());
    }
}

void MailListModel::setFilter(const QVariantMap &filter)
{
    qDebug() << "MailListModel::setFilter " << filter;
    using namespace Sink;
    using namespace Sink::ApplicationDomain;
    bool validQuery = false;
    Sink::Query query;

    //Base queries
    if (filter.value("folder").value<Folder::Ptr>()) {
        auto folder = filter.value("folder").value<Folder::Ptr>();
        const auto specialPurpose = folder->getSpecialPurpose();
        mIsThreaded = !(specialPurpose.contains(SpecialPurpose::Mail::drafts) ||
                                specialPurpose.contains(SpecialPurpose::Mail::sent));

        query = [&] {
            if (mIsThreaded) {
                return Sink::StandardQueries::threadLeaders(*folder);
            } else {
                Sink::Query query;
                query.setId("threadleaders-unthreaded");
                if (!folder->resourceInstanceIdentifier().isEmpty()) {
                    query.resourceFilter(folder->resourceInstanceIdentifier());
                }
                query.filter<Sink::ApplicationDomain::Mail::Folder>(*folder);
                query.sort<Sink::ApplicationDomain::Mail::Date>();
                return query;
            }
        }();
        if (!folder->getSpecialPurpose().contains(Sink::ApplicationDomain::SpecialPurpose::Mail::trash)) {
            //Filter trash if this is not a trash folder
            query.filter<Sink::ApplicationDomain::Mail::Trash>(false);
        }

        query.setFlags(Sink::Query::LiveQuery);
        query.limit(100);

        qDebug() << "Running folder query: " << folder->resourceInstanceIdentifier() << folder->identifier();
        //Latest mail on top
        sort(0, Qt::DescendingOrder);
        validQuery = true;
    } else if (!filter.value("account").toByteArray().isEmpty()) {
        query.resourceFilter<SinkResource::Account>(filter.value("account").toByteArray());
    }

    if (filter.value("important").toBool()) {
        query.setId("threadLeadersImportant");
        query.setFlags(Sink::Query::LiveQuery);
        query.filter<Sink::ApplicationDomain::Mail::Important>(true);
        query.sort<ApplicationDomain::Mail::Date>();
        query.reduce<ApplicationDomain::Mail::ThreadId>(Query::Reduce::Selector::max<ApplicationDomain::Mail::Date>())
            .count()
            .select<ApplicationDomain::Mail::Subject>(Query::Reduce::Selector::Min)
            .collect<ApplicationDomain::Mail::Unread>()
            .collect<ApplicationDomain::Mail::Important>();
        //Latest mail at the top
        sort(0, Qt::DescendingOrder);
        validQuery = true;
    }

    if (filter.value("drafts").toBool()) {
        query.setFlags(Sink::Query::LiveQuery);
        query.filter<Mail::Draft>(true);
        query.filter<Mail::Trash>(false);
        qDebug() << "Running mail query for drafts: ";
        //Latest mail at the top
        sort(0, Qt::DescendingOrder);
        validQuery = true;
    }
    if (filter.value("inbox").toBool()) {
        Sink::Query folderQuery{};
        folderQuery.containsFilter<Sink::ApplicationDomain::Folder::SpecialPurpose>(Sink::ApplicationDomain::SpecialPurpose::Mail::inbox);
        folderQuery.request<Sink::ApplicationDomain::Folder::SpecialPurpose>();
        folderQuery.request<Sink::ApplicationDomain::Folder::Name>();

        query.setFlags(Sink::Query::LiveQuery);
        query.filter<Sink::ApplicationDomain::Mail::Folder>(folderQuery);
        query.sort<Mail::Date>();
        //Latest mail at the top
        sort(0, Qt::DescendingOrder);
        validQuery = true;
    }

    if (filter.contains("singleMail") && filter.value("singleMail").value<Sink::ApplicationDomain::Mail::Ptr>()) {
        auto mail = filter.value("singleMail").value<Sink::ApplicationDomain::Mail::Ptr>();
        query = Sink::Query{*mail};
        query.setFlags(Sink::Query::LiveQuery | Sink::Query::UpdateStatus);
        //Latest mail at the bottom
        sort(0, Qt::AscendingOrder);
        validQuery = true;
    }
    if (filter.contains("mail") && filter.value("mail").value<Sink::ApplicationDomain::Mail::Ptr>()) {
        auto mail = filter.value("mail").value<Sink::ApplicationDomain::Mail::Ptr>();
        query = Sink::StandardQueries::completeThread(*mail);
        query.setFlags(Sink::Query::LiveQuery | Sink::Query::UpdateStatus);
        //Latest mail at the bottom
        sort(0, Qt::AscendingOrder);
        validQuery = true;
    }

    //Additional filtering
    if (filter.contains("string") && filter.value("string").isValid()) {
        const auto filterString = filter.value("string").toString();
        if (filterString.length() < 3 && !filterString.isEmpty()) {
            return;
        }
        if (!filterString.isEmpty()) {
            query.filter({}, Sink::QueryBase::Comparator(filterString, Sink::QueryBase::Comparator::Fulltext));
            query.limit(0);
        }
        validQuery = true;
    }

    if (filter.value("hideTrash").toBool()) {
        query.filter<Sink::ApplicationDomain::Mail::Trash>(false);
    }

    if (filter.value("hideNonTrash").toBool()) {
        query.filter<Sink::ApplicationDomain::Mail::Trash>(true);
    }

    if (filter.value("headersOnly").toBool()) {
        requestHeaders(query);
    } else {
        requestFullMail(query);
    }

    mFetchMails = filter.value("fetchMails").toBool();
    //TODO don't reset on string filter update?
    mFetchedMails.clear();

    if (validQuery) {
        runQuery(query);
    } else {
        setSourceModel(nullptr);
    }
}

QVariantMap MailListModel::filter() const
{
    return {};
}

