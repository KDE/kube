/*
    Copyright (c) 2018 Christian Mollekopf <mollekopf@kolabsys.com>

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
#include "domainobjectcontroller.h"

#include <sink/store.h>
#include <sink/applicationdomaintype.h>
#include <kmime/kmime_message.h>
#include <mailtemplates.h>

using namespace Kube;

DomainObjectController::DomainObjectController(QObject *parent)
    : QObject(parent)
{

}

QStringList toStringList(const QVariantList &list)
{
    QStringList s;
    for (const auto &e : list) {
        s << e.toString();
    }
    return s;
}

void DomainObjectController::create(const QVariantMap &object)
{
    using namespace Sink::ApplicationDomain;
    qWarning() << "Creating " << object;
    auto type = object["type"].toString();
    if (type == getTypeName<Mail>()) {
        auto toAddresses = toStringList(object["to"].toList());
        auto ccAddresses = toStringList(object["cc"].toList());
        auto bccAddresses = toStringList(object["bcc"].toList());

        KMime::Types::Mailbox mb;
        mb.fromUnicodeString("identity@example.org");
        auto msg = MailTemplates::createMessage({},
                toAddresses,
                ccAddresses,
                bccAddresses,
                mb,
                object["subject"].toString(),
                object["body"].toString(),
                {},
                {},
                {},
                {});

        Sink::Query query;
        query.containsFilter<SinkResource::Capabilities>(ResourceCapabilities::Mail::storage);
        query.filter<SinkResource::Account>("account1");
        Sink::Store::fetchAll<SinkResource>(query)
            .then([=](const QList<SinkResource::Ptr> &resources) {
                if (!resources.isEmpty()) {
                    auto resourceId = resources[0]->identifier();
                    qWarning() << "Using resource " << resourceId << " from account " << resources[0]->getAccount();
                    auto mail = ApplicationDomainType::createEntity<Mail>(resourceId);
                    mail.setMimeMessage(msg->encodedContent(true));
                    Sink::Store::create<Mail>(mail).exec();
                    monitor(QVariant::fromValue(Mail::Ptr::create(mail)));
                } else {
                    qWarning() << "No resources found.";
                }
            }).exec();
    }

}

void DomainObjectController::monitor(const QVariant &object)
{
    using namespace Sink::ApplicationDomain;
    qWarning() << "Monitoring " << object;
    auto mail = object.value<Mail::Ptr>();
    Q_ASSERT(mail);
    Sink::Query query{*mail};
    query.setFlags(Sink::Query::LiveQuery);
    mModel = Sink::Store::loadModel<Mail>(query);
    QObject::connect(mModel.data(), &QAbstractItemModel::rowsInserted, [=](const QModelIndex &index, int start, int end) {
        for (int i = start; i <= end; i++) {
            auto mail = mModel->index(i, 0, QModelIndex()).data(Sink::Store::DomainObjectRole).value<Mail::Ptr>();
            mCurrentObject = QVariant::fromValue(mail);
            emit currentObjectChanged();
            break;
        }
    });
}

QVariant DomainObjectController::currentObject() const
{
    return mCurrentObject;
}
