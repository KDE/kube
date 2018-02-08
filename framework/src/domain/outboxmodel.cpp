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
#include <QString>

#include <sink/standardqueries.h>
#include <sink/notifier.h>
#include <sink/notification.h>


OutboxModel::OutboxModel(QObject *parent)
    : QSortFilterProxyModel(parent),
    mNotifier(new Sink::Notifier{Sink::Query{}.containsFilter<Sink::ApplicationDomain::SinkResource::Capabilities>(Sink::ApplicationDomain::ResourceCapabilities::Mail::transport)}),
    mStatus(NoStatus)
{
    setDynamicSortFilter(true);
    sort(0, Qt::DescendingOrder);

    using namespace Sink::ApplicationDomain;
    auto query = Sink::StandardQueries::outboxMails();
    query.setFlags(Sink::Query::LiveQuery | Sink::Query::UpdateStatus);
    query.request<Mail::Subject>();
    query.request<Mail::Date>();
    query.request<Mail::Folder>();
    runQuery(query);
    connect(this, &QAbstractItemModel::rowsInserted, this, &OutboxModel::countChanged);
    connect(this, &QAbstractItemModel::rowsRemoved, this, &OutboxModel::countChanged);

    mNotifier->registerHandler([this] (const Sink::Notification &n) {
        //TODO aggregate status from multiple resources
        if (n.type == Sink::Notification::Status) {
            switch (n.code) { 
                case Sink::ApplicationDomain::Status::ErrorStatus:
                    mStatus = ErrorStatus;
                    break;
                case Sink::ApplicationDomain::Status::BusyStatus:
                    mStatus = InProgressStatus;
                    break;
                default:
                    mStatus = NoStatus;
                    break;
            }
            emit statusChanged();
        }

    });

}

OutboxModel::~OutboxModel()
{

}

QHash< int, QByteArray > OutboxModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[Subject] = "subject";
    roles[Date] = "date";
    roles[Status] = "status";
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
        case Date:
            return mail->getDate();
        case Status: {
            const auto status = srcIdx.data(Sink::Store::StatusRole).toInt();
            if (status == Sink::ApplicationDomain::SyncStatus::SyncInProgress) {
                return InProgressStatus;
            }
            if (status == Sink::ApplicationDomain::SyncStatus::SyncError) {
                return ErrorStatus;
            }
            return PendingStatus;
        }
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
    mModel = Sink::Store::loadModel<Sink::ApplicationDomain::Mail>(query);
    setSourceModel(mModel.data());
}

int OutboxModel::count() const
{
    return rowCount();
}

int OutboxModel::status() const
{
    return mStatus;
}
