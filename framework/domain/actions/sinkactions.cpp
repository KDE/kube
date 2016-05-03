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

using namespace Kube;

static ActionHandlerHelper markAsReadHandler("org.kde.kube.actions.mark-as-read",
    [](Context *context) -> bool {
        return context->property("mail").isValid();
    },
    [](Context *context) {
        auto mail = context->property("mail").value<Sink::ApplicationDomain::Mail::Ptr>();
        if (!mail) {
            qWarning() << "Failed to get the mail mail: " << context->property("mail");
            return;
        }
        mail->setProperty("unread", false);
        qDebug() << "Mark as read " << mail->identifier();
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
            qWarning() << "Failed to get the mail mail: " << context->property("mail");
            return;
        }
        mail->setProperty("unread", false);
        qDebug() << "Remove " << mail->identifier();
        Sink::Store::remove(*mail).exec();
    }
);

static ActionHandlerHelper synchronizeHandler("org.kde.kube.actions.synchronize",
    [](Context *context) -> bool {
        return true;
    },
    [](Context *context) {
        if (auto folder = context->property("folder").value<Sink::ApplicationDomain::Folder::Ptr>()) {
            qDebug() << "Synchronizing resource " << folder->resourceInstanceIdentifier();
            Sink::Store::synchronize(Sink::Query::ResourceFilter(folder->resourceInstanceIdentifier())).exec();
        } else {
            qDebug() << "Synchronizing all";
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
        qWarning() << "Sending a mail: ";

        Sink::Query query;
        query += Sink::Query::PropertyFilter("type", "org.kde.mailtransport");
        query += Sink::Query::PropertyFilter("account", QVariant::fromValue(accountId));
        Sink::Store::fetchAll<Sink::ApplicationDomain::SinkResource>(query)
            .then<void, QList<Sink::ApplicationDomain::SinkResource::Ptr>>([=](const QList<Sink::ApplicationDomain::SinkResource::Ptr> &resources) {
                if (!resources.isEmpty()) {
                    auto resourceId = resources[0]->identifier();
                    qDebug() << "Sending message via resource: " << resourceId;
                    Sink::ApplicationDomain::Mail mail(resourceId);
                    mail.setBlobProperty("mimeMessage", message->encodedContent());
                    return Sink::Store::create(mail);
                }
                qWarning() << "Failed to find a mailtransport resource";
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
        qWarning() << "executing save as draft";
        const auto accountId = context->property("accountId").value<QByteArray>();
        const auto message = context->property("message").value<KMime::Message::Ptr>();
        auto existingMail = context->property("existingMail").value<Sink::ApplicationDomain::Mail>();
        if (!message) {
            qWarning() << "Failed to get the mail: " << context->property("mail");
            return KAsync::error<void>(1, "Failed to get the mail: " + context->property("mail").toString());
        }

        if (existingMail.identifier().isEmpty()) {
            Sink::Query query;
            query += Sink::Query::RequestedProperties(QByteArrayList() << "name");
            query += Sink::Query::PropertyContainsFilter("specialpurpose", "drafts");
            query += Sink::Query::AccountFilter(accountId);
            qWarning() << "fetching the drafts folder";
            return Sink::Store::fetchAll<Sink::ApplicationDomain::Folder>(query)
                .then<void, QList<Sink::ApplicationDomain::Folder::Ptr>>([=](const QList<Sink::ApplicationDomain::Folder::Ptr> folders) {
                    qWarning() << "fetched a drafts folder" << folders.size();
                    if (folders.isEmpty()) {
                        return KAsync::error<void>(1, "Failed to find a drafts folder.");
                    }
                    if (folders.size() > 1) {
                        qWarning() << "Found too many draft folders (taking the first): " << folders;
                    }
                    const auto folder = folders.first();
                    Sink::ApplicationDomain::Mail mail(folder->resourceInstanceIdentifier());
                    mail.setProperty("folder", folder->identifier());
                    mail.setBlobProperty("mimeMessage", message->encodedContent());
                    return Sink::Store::create(mail);
                })
                .then<void>([](){
                    qWarning() << "done";
                });
        } else {
            qWarning() << "Modifying an existing mail";
            existingMail.setBlobProperty("mimeMessage", message->encodedContent());
            return Sink::Store::modify(existingMail);
        }
    })
);
