/*
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
#include "sinkutils.h"

#include <QTemporaryFile>
#include <sink/store.h>
#include <sink/log.h>

KAsync::Job<void> SinkUtils::sendMail(const QByteArray &messageContent, const QByteArray &accountId)
{
    using namespace Sink;
    using namespace Sink::ApplicationDomain;

    SinkLog() << "Sending message via account: " << accountId;
    Query query;
    query.containsFilter<SinkResource::Capabilities>(ResourceCapabilities::Mail::transport);
    query.filter<SinkResource::Account>(accountId);
    return Store::fetchAll<SinkResource>(query)
        .then([=](const QList<SinkResource::Ptr> &resources) {
            if (!resources.isEmpty()) {
                auto resourceId = resources[0]->identifier();
                SinkLog() << "Sending message via resource: " << resourceId;
                Mail mail(resourceId);
                mail.setMimeMessage(messageContent);
                return Store::create(mail)
                    .then<void>([=] {
                        //Trigger a sync, but don't wait for it.
                        Store::synchronize(Sink::SyncScope{}.resourceFilter(resourceId)).exec();
                    });
            }
            SinkWarning() << "Failed to find a mailtransport resource";
            return KAsync::error("Failed to find a MailTransport resource.");
        })
        .then([&] (const KAsync::Error &error) {
            if (error) {
                QTemporaryFile tmp;
                tmp.setAutoRemove(false);
                if (tmp.open()) {
                    tmp.write(messageContent);
                    tmp.close();
                    SinkWarning() << "Saved your message contents to: " << tmp.fileName();
                }
                SinkError() << "Failed to send the message: " << error;
            } else {
                SinkLog() << "Message was sent.";
            }
            return error;
        });
}
