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
using namespace Sink;
using namespace Sink::ApplicationDomain;

static ActionHandlerHelper markAsReadHandler("org.kde.kube.actions.mark-as-read",
    [](Context *context) -> bool {
        return context->property("mail").isValid();
    },
    [](Context *context) {
        auto mail = context->property("mail").value<Mail::Ptr>();
        if (!mail) {
            SinkWarning() << "Failed to get the mail mail: " << context->property("mail");
            return;
        }
        mail->setProperty("unread", false);
        SinkLog() << "Mark as read " << mail->identifier();
        Store::modify(*mail).exec();
    }
);

static ActionHandlerHelper moveToTrashHandler("org.kde.kube.actions.move-to-trash",
    [](Context *context) -> bool {
        return context->property("mail").isValid();
    },
    [](Context *context) {
        auto mail = context->property("mail").value<Mail::Ptr>();
        if (!mail) {
            SinkWarning() << "Failed to get the mail mail: " << context->property("mail");
            return;
        }
        mail->setTrash(true);
        SinkLog() << "Move to trash " << mail->identifier();
        Store::modify(*mail).exec();
    }
);

static ActionHandlerHelper deleteHandler("org.kde.kube.actions.delete",
    [](Context *context) -> bool {
        return context->property("mail").isValid();
    },
    [](Context *context) {
        auto mail = context->property("mail").value<Mail::Ptr>();
        if (!mail) {
            SinkWarning() << "Failed to get the mail mail: " << context->property("mail");
            return;
        }
        SinkLog() << "Remove " << mail->identifier();
        Store::remove(*mail).exec();
    }
);

static ActionHandlerHelper synchronizeHandler("org.kde.kube.actions.synchronize",
    [](Context *context) -> bool {
        return true;
    },
    [](Context *context) {
        if (auto folder = context->property("folder").value<Folder::Ptr>()) {
            SinkLog() << "Synchronizing folder " << folder->resourceInstanceIdentifier() << folder->identifier();
            auto scope = SyncScope().resourceFilter(folder->resourceInstanceIdentifier()).filter<Mail::Folder>(QVariant::fromValue(folder->identifier()));
            scope.setType<ApplicationDomain::Mail>();
            Store::synchronize(scope).exec();
        } else {
            SinkLog() << "Synchronizing all";
            Store::synchronize(SyncScope()).exec();
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

        Query query;
        query.containsFilter<ApplicationDomain::SinkResource::Capabilities>(ApplicationDomain::ResourceCapabilities::Mail::transport);
        query.filter<SinkResource::Account>(accountId);
        Store::fetchAll<ApplicationDomain::SinkResource>(query)
            .then<void, QList<ApplicationDomain::SinkResource::Ptr>>([=](const QList<ApplicationDomain::SinkResource::Ptr> &resources) -> KAsync::Job<void> {
                if (!resources.isEmpty()) {
                    auto resourceId = resources[0]->identifier();
                    SinkTrace() << "Sending message via resource: " << resourceId;
                    Mail mail(resourceId);
                    mail.setBlobProperty("mimeMessage", message->encodedContent());
                    return Store::create(mail);
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
        SinkLog() << "Executing the save-as-draft action";
        const auto accountId = context->property("accountId").value<QByteArray>();
        const auto message = context->property("message").value<KMime::Message::Ptr>();
        auto existingMail = context->property("existingMail").value<Mail>();
        if (!message) {
            SinkWarning() << "Failed to get the mail: " << context->property("mail");
            return KAsync::error<void>(1, "Failed to get the mail: " + context->property("mail").toString());
        }

        if (existingMail.identifier().isEmpty()) {
            Query query;
            query.containsFilter<SinkResource::Capabilities>(ApplicationDomain::ResourceCapabilities::Mail::drafts);
            query.filter<SinkResource::Account>(accountId);
            return Store::fetchOne<SinkResource>(query)
                .then<void, SinkResource>([=](const SinkResource &resource) -> KAsync::Job<void> {
                    Mail mail(resource.identifier());
                    mail.setDraft(true);
                    mail.setMimeMessage(message->encodedContent());
                    return Store::create(mail);
                });
        } else {
            SinkWarning() << "Modifying an existing mail" << existingMail.identifier();
            existingMail.setMimeMessage(message->encodedContent());
            return Store::modify(existingMail);
        }
    })
);
