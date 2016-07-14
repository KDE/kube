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
#include <actions/context.h>
#include <actions/actionhandler.h>

#include <KMime/Message>
#include <QFile>

#include <sink/store.h>
#include <sink/log.h>

SINK_DEBUG_AREA("sinkactions")

using namespace Kube;

static ActionHandlerHelper markAsReadHandler("org.kde.kube.actions.mark-as-read",
    [](Context *context) -> bool {
        return context->property("mail").isValid();
    },
    [](Context *context) {
        auto mail = context->property("mail").value<Sink::ApplicationDomain::Mail::Ptr>();
        if (!mail) {
            SinkWarning() << "Failed to get the mail mail: " << context->property("mail");
            return;
        }
        mail->setProperty("unread", false);
        SinkLog() << "Mark as read " << mail->identifier();
        Sink::Store::modify(*mail).exec();
    }
);

static ActionHandlerHelper moveToTrashHandler("org.kde.kube.actions.move-to-trash",
    [](Context *context) -> bool {
        return context->property("mail").isValid();
    },
    [](Context *context) {
        auto mail = context->property("mail").value<Sink::ApplicationDomain::Mail::Ptr>();
        if (!mail) {
            SinkWarning() << "Failed to get the mail mail: " << context->property("mail");
            return;
        }
        mail->setTrash(true);
        SinkLog() << "Move to trash " << mail->identifier();
        Sink::Store::modify(*mail).exec();
    }
);

static ActionHandlerHelper deleteHandler("org.kde.kube.actions.delete",
    [](Context *context) -> bool {
        return context->property("mail").isValid();
    },
    [](Context *context) {
        auto mail = context->property("mail").value<Sink::ApplicationDomain::Mail::Ptr>();
        if (!mail) {
            SinkWarning() << "Failed to get the mail mail: " << context->property("mail");
            return;
        }
        SinkLog() << "Remove " << mail->identifier();
        Sink::Store::remove(*mail).exec();
    }
);

static ActionHandlerHelper synchronizeHandler("org.kde.kube.actions.synchronize",
    [](Context *context) -> bool {
        return true;
    },
    [](Context *context) {
        if (auto folder = context->property("folder").value<Sink::ApplicationDomain::Folder::Ptr>()) {
            SinkLog() << "Synchronizing resource " << folder->resourceInstanceIdentifier();
            Sink::Store::synchronize(Sink::Query::ResourceFilter(folder->resourceInstanceIdentifier())).exec();
        } else {
            SinkLog() << "Synchronizing all";
            Sink::Store::synchronize(Sink::Query()).exec();
        }
    }
);

static ActionHandlerHelper sendMailHandler("org.kde.kube.actions.sendmail",
    [](Context *context) -> bool {
        auto accountId = context->property("accountId").value<QByteArray>();
        auto message = context->property("message").value<KMime::Message::Ptr>();
        return !accountId.isEmpty() && message;
    },
    [](Context *context) {
        auto accountId = context->property("accountId").value<QByteArray>();
        auto message = context->property("message").value<KMime::Message::Ptr>();
        SinkLog() << "Sending a mail: ";

        Sink::Query query;
        query += Sink::Query::CapabilityFilter(Sink::ApplicationDomain::ResourceCapabilities::Mail::transport);
        query += Sink::Query::AccountFilter(accountId);
        Sink::Store::fetchAll<Sink::ApplicationDomain::SinkResource>(query)
            .then<void, KAsync::Job<void>, QList<Sink::ApplicationDomain::SinkResource::Ptr>>([=](const QList<Sink::ApplicationDomain::SinkResource::Ptr> &resources) -> KAsync::Job<void> {
                if (!resources.isEmpty()) {
                    auto resourceId = resources[0]->identifier();
                    SinkTrace() << "Sending message via resource: " << resourceId;
                    Sink::ApplicationDomain::Mail mail(resourceId);
                    mail.setBlobProperty("mimeMessage", message->encodedContent());
                    return Sink::Store::create(mail);
                }
                SinkWarning() << "Failed to find a mailtransport resource";
                return KAsync::error<void>(0, "Failed to find a MailTransport resource.");
            }).exec();
    }
);

static ActionHandlerHelper saveAsDraft("org.kde.kube.actions.save-as-draft",
    [](Context *context) -> bool {
        auto accountId = context->property("accountId").value<QByteArray>();
        auto message = context->property("message").value<KMime::Message::Ptr>();
        return !accountId.isEmpty() && message;
    },
    ActionHandlerHelper::JobHandler([](Context *context) -> KAsync::Job<void> {
        SinkWarning() << "executing save as draft";
        const auto accountId = context->property("accountId").value<QByteArray>();
        const auto message = context->property("message").value<KMime::Message::Ptr>();
        auto existingMail = context->property("existingMail").value<Sink::ApplicationDomain::Mail>();
        if (!message) {
            SinkWarning() << "Failed to get the mail: " << context->property("mail");
            return KAsync::error<void>(1, "Failed to get the mail: " + context->property("mail").toString());
        }

        if (existingMail.identifier().isEmpty()) {
            Sink::Query query;
            query += Sink::Query::CapabilityFilter(Sink::ApplicationDomain::ResourceCapabilities::Mail::drafts);
            query += Sink::Query::AccountFilter(accountId);
            return Sink::Store::fetchOne<Sink::ApplicationDomain::SinkResource>(query)
                .then<void, KAsync::Job<void>, Sink::ApplicationDomain::SinkResource>([=](const Sink::ApplicationDomain::SinkResource &resource) -> KAsync::Job<void> {
                    Sink::ApplicationDomain::Mail mail(resource.identifier());
                    mail.setProperty("draft", true);
                    mail.setBlobProperty("mimeMessage", message->encodedContent());
                    return Sink::Store::create(mail);
                });
        } else {
            SinkWarning() << "Modifying an existing mail" << existingMail.identifier();
            existingMail.setBlobProperty("mimeMessage", message->encodedContent());
            return Sink::Store::modify(existingMail);
        }
    })
);
