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
#include "extensionapi.h"

#include <mailtemplates.h>
#include <KMime/KMimeMessage>
#include <sink/store.h>
#include <sink/log.h>

#include <QDebug>

static void send(const QByteArray &message, const QByteArray &accountId)
{
    using namespace Sink;
    using namespace Sink::ApplicationDomain;

    Q_ASSERT(!accountId.isEmpty());
    Query query;
    query.containsFilter<SinkResource::Capabilities>(ResourceCapabilities::Mail::transport);
    query.filter<SinkResource::Account>(accountId);
    auto job = Store::fetchAll<SinkResource>(query)
        .then([=](const QList<SinkResource::Ptr> &resources) {
            if (!resources.isEmpty()) {
                auto resourceId = resources[0]->identifier();
                SinkLog() << "Sending message via resource: " << resourceId;
                Mail mail(resourceId);
                mail.setMimeMessage(message);
                return Store::create(mail)
                    .then<void>([=] {
                        //Trigger a sync, but don't wait for it.
                        Store::synchronize(Sink::SyncScope{}.resourceFilter(resourceId)).exec();
                    });
            }
            SinkWarning() << "Failed to find a mailtransport resource";
            return KAsync::error<void>(0, "Failed to find a MailTransport resource.");
        })
        .then([&] (const KAsync::Error &) {
            SinkLog() << "Message was sent: ";
        });
    job.exec();
}

static QStringList toStringList(const QVariantList &list)
{
    QStringList s;
    for (const auto &e : list) {
        s << e.toString();
    }
    return s;
}

Q_INVOKABLE void ExtensionApi::forwardMail(const QVariantMap &map)
{
    SinkLog() << "Forwarding mail " << map;
    auto mailObject = map.value("mail").value<Sink::ApplicationDomain::Mail::Ptr>();
    Q_ASSERT(mailObject);
    KMime::Message::Ptr msg(new KMime::Message);
    msg->setContent(KMime::CRLFtoLF(mailObject->getMimeMessage()));
    msg->parse();

    MailTemplates::forward(msg, [map] (const KMime::Message::Ptr &fwdMessage) {
        auto msg = fwdMessage;
        msg->subject()->fromUnicodeString(map.value("subject").toString(), "utf8");
        auto list = toStringList(map.value("to").toList());
        for (const auto &address : list) {
            KMime::Types::Mailbox mb;
            mb.fromUnicodeString(address);
            msg->to()->addAddress(mb);
        }
        msg->assemble();
        send(msg->encodedContent(true), map.value("accountId").toByteArray());
    });
}
