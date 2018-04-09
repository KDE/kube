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
#include "peoplemodel.h"

#include <sink/standardqueries.h>
#include <sink/store.h>

PeopleModel::PeopleModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    using namespace Sink::ApplicationDomain;

    setDynamicSortFilter(true);
    sort(0, Qt::DescendingOrder);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    Sink::Query query;
    query.setFlags(Sink::Query::LiveQuery);
    query.request<Contact::Fn>();
    query.request<Contact::Emails>();
    query.request<Contact::Addressbook>();
    query.request<Contact::Vcard>();
    query.request<Contact::Firstname>();
    query.request<Contact::Lastname>();
    query.request<Contact::Photo>();
    runQuery(query);
}

PeopleModel::~PeopleModel()
{

}

void PeopleModel::setFilter(const QString &filter)
{
    setFilterWildcard(filter);
}

QString PeopleModel::filter() const
{
     return {};
}

QHash< int, QByteArray > PeopleModel::roleNames() const
{
    static QHash<int, QByteArray> roles = {
        {Name, "name"},
        {Emails, "emails"},
        {Addressbook, "addressbook"},
        {Type, "type"},
        {DomainObject, "domainObject"},
        {FirstName, "firstName"},
        {LastName, "lastName"},
        {ImageData, "imageData"}
        };
    return roles;
}

static QStringList toStringList(const QList<Sink::ApplicationDomain::Contact::Email> &list)
{
    QStringList out;
    std::transform(list.constBegin(), list.constEnd(), std::back_inserter(out), [] (const Sink::ApplicationDomain::Contact::Email &s) { return s.email; });
    return out;
}

QPair<QString, QString> getFirstnameLastname(const QString &fn)
{
    auto parts = fn.split(' ');
    if (parts.isEmpty()) {
        return {};
    }
    if (parts.size() == 1) {
        return {parts.first(), {}};
    }
    const auto lastName = parts.takeLast();
    return {parts.join(' '), lastName};
}

QVariant PeopleModel::data(const QModelIndex &idx, int role) const
{
    auto srcIdx = mapToSource(idx);
    auto contact = srcIdx.data(Sink::Store::DomainObjectRole).value<Sink::ApplicationDomain::Contact::Ptr>();
    switch (role) {
        case Name:
            return contact->getFn();
        case Emails:
            return QVariant::fromValue(toStringList(contact->getEmails()));
        case Addressbook:
            return contact->getAddressbook();
        case Type:
            return "contact";
        case DomainObject:
            return QVariant::fromValue(contact);
        case FirstName: {
            const auto n = contact->getFirstname();
            //Fall back to the fn if we have no name
            if (n.isEmpty()) {
                return getFirstnameLastname(contact->getFn()).first;
            }
            return n;
        }
        case LastName: {
            const auto n = contact->getLastname();
            //Fall back to the fn if we have no name
            if (n.isEmpty()) {
                return getFirstnameLastname(contact->getFn()).second;
            }
            return n;
        }
        case ImageData:
            return contact->getPhoto();
    }
    return QSortFilterProxyModel::data(idx, role);
}

bool PeopleModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    const auto leftName = left.data(Sink::Store::DomainObjectRole).value<Sink::ApplicationDomain::Contact::Ptr>()->getFn();
    const auto rightName = right.data(Sink::Store::DomainObjectRole).value<Sink::ApplicationDomain::Contact::Ptr>()->getFn();
    return leftName < rightName;
}

void PeopleModel::runQuery(const Sink::Query &query)
{
    mModel = Sink::Store::loadModel<Sink::ApplicationDomain::Contact>(query);
    setSourceModel(mModel.data());
}

void PeopleModel::setAddressbook(const QVariant &/*parentFolder*/)
{
    //TODO filter query by addressbook
    qWarning() << "The addressbook filter is not yet implemented";
}

QVariant PeopleModel::addressbook() const
{
    return QVariant();
}


