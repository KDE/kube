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
#include "teststore.h"

#include <sink/store.h>
#include <sink/resourcecontrol.h>
#include <sink/secretstore.h>
#include <kmime/kmime_message.h>

#include <QDebug>
#include <QVariant>

#include "framework/src/domain/mime/mailtemplates.h"

using namespace Kube;

static void iterateOverObjects(const QVariantList &list, std::function<void(const QVariantMap &)> callback)
{
    for (const auto &entry : list) {
        auto object = entry.toMap();
        callback(object);
    }
}

static QStringList toStringList(const QVariantList &list)
{
    QStringList s;
    for (const auto &e : list) {
        s << e.toString();
    }
    return s;
}

static QByteArrayList toByteArrayList(const QVariantList &list)
{
    QByteArrayList s;
    for (const auto &e : list) {
        s << e.toByteArray();
    }
    return s;
}

static void createMail(const QVariantMap &object, const QByteArray &folder = {})
{
    using namespace Sink::ApplicationDomain;

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
    if (object.contains("messageId")) {
        msg->messageID(true)->from7BitString(object["messageId"].toByteArray());
    }
    if (object.contains("inReplyTo")) {
        msg->inReplyTo(true)->from7BitString(object["inReplyTo"].toByteArray());
    }
    if (object.contains("date")) {
        msg->date(true)->setDateTime(QDateTime::fromString(object["date"].toString(), Qt::ISODate));
    }

    msg->assemble();

    auto mail = ApplicationDomainType::createEntity<Mail>(object["resource"].toByteArray());
    mail.setMimeMessage(msg->encodedContent(true));
    if (!folder.isEmpty()) {
        mail.setFolder(folder);
    }
    Sink::Store::create(mail).exec().waitForFinished();
}

static void createFolder(const QVariantMap &object)
{
    using namespace Sink::ApplicationDomain;
    auto folder = ApplicationDomainType::createEntity<Folder>(object["resource"].toByteArray());
    folder.setName(object["name"].toString());
    folder.setSpecialPurpose(toByteArrayList(object["specialpurpose"].toList()));
    Sink::Store::create(folder).exec().waitForFinished();

    iterateOverObjects(object.value("mails").toList(), [=](const QVariantMap &object) {
        createMail(object, folder.identifier());
    });
}

void TestStore::setup(const QVariantMap &map)
{
    using namespace Sink::ApplicationDomain;
    iterateOverObjects(map.value("accounts").toList(), [&] (const QVariantMap &object) {
        auto account = ApplicationDomainType::createEntity<SinkAccount>("", object["id"].toByteArray());
        account.setName(object["name"].toString());
        Sink::Store::create(account).exec().waitForFinished();
    });
    QByteArrayList resources;
    iterateOverObjects(map.value("resources").toList(), [&] (const QVariantMap &object) {
        resources << object["id"].toByteArray();
        auto resource = [&] {
            using namespace Sink::ApplicationDomain;
            auto resource = ApplicationDomainType::createEntity<SinkResource>("", object["id"].toByteArray());
            if (object["type"] == "dummy") {
                resource.setResourceType("sink.dummy");
            } else if (object["type"] == "mailtransport") {
                resource.setResourceType("sink.mailtransport");
            } else {
                Q_ASSERT(false);
            }
            return resource;
        }();
        resource.setAccount(object["account"].toByteArray());
        Sink::Store::create(resource).exec().waitForFinished();
        Sink::SecretStore::instance().insert(resource.identifier(), "secret");
    });

    iterateOverObjects(map.value("identities").toList(), [] (const QVariantMap &object) {
        auto identity = Sink::ApplicationDomain::Identity{};
        identity.setAccount(object["account"].toByteArray());
        identity.setAddress(object["address"].toString());
        identity.setName(object["name"].toString());
        Sink::Store::create(identity).exec().waitForFinished();
    });

    iterateOverObjects(map.value("folders").toList(), createFolder);
    iterateOverObjects(map.value("mails").toList(), [] (const QVariantMap &map) {
        createMail(map);
    });

    Sink::ResourceControl::flushMessageQueue(resources).exec().waitForFinished();
}

QVariant TestStore::load(const QByteArray &type, const QVariantMap &filter)
{
    using namespace Sink::ApplicationDomain;
    if (type == "mail") {
        Sink::Query query;
        if (filter.contains("resource")) {
            query.resourceFilter(filter.value("resource").toByteArray());
        }
        auto list = Sink::Store::read<Mail>(query);
        if (!list.isEmpty()) {
            return QVariant::fromValue(Mail::Ptr::create(list.first()));
        }
        return {};
    }

    Q_ASSERT(false);
    return {};
}
