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
#include "sinkfabric.h"

#include <QFile>

#include <sink/store.h>
#include <sink/log.h>
#include <sink/notification.h>
#include <sink/notifier.h>
#include <sink/secretstore.h>

#include "fabric.h"
#include "keyring.h"

using namespace Kube;
using namespace Sink;
using namespace Sink::ApplicationDomain;

class SinkListener : public Kube::Fabric::Listener
{
public:
    SinkListener() = default;

    void notify(const QString &id, const QVariantMap &message)
    {
        SinkLog() << "Received message: " << id << message;
        if (id == "synchronize"/*Kube::Messages::synchronize*/) {
            if (auto folder = message["folder"].value<ApplicationDomain::Folder::Ptr>()) {
                SinkLog() << "Synchronizing folder " << folder->resourceInstanceIdentifier() << folder->identifier();
                auto scope = SyncScope().resourceFilter(folder->resourceInstanceIdentifier()).filter<Mail::Folder>(QVariant::fromValue(folder->identifier()));
                scope.setType<ApplicationDomain::Mail>();
                Store::synchronize(scope).exec();
            } else if (message.contains("specialPurpose")) {
                auto specialPurpose = message["specialPurpose"].value<QString>();
                //Synchronize all drafts folders
                if (specialPurpose == "drafts") {
                    //TODO or rather just synchronize draft mails and have resource figure out what that means?
                    Sink::Query folderQuery{};
                    folderQuery.containsFilter<Sink::ApplicationDomain::Folder::SpecialPurpose>(Sink::ApplicationDomain::SpecialPurpose::Mail::drafts);
                    folderQuery.request<Sink::ApplicationDomain::Folder::SpecialPurpose>();
                    folderQuery.request<Sink::ApplicationDomain::Folder::Name>();
                    Store::fetch<Sink::ApplicationDomain::Folder>(folderQuery)
                        .then([] (const QList<Sink::ApplicationDomain::Folder::Ptr> &folders) {
                            for (const auto f : folders) {
                                auto scope = SyncScope().resourceFilter(f->resourceInstanceIdentifier()).filter<Mail::Folder>(QVariant::fromValue(f->identifier()));
                                scope.setType<ApplicationDomain::Mail>();
                                Store::synchronize(scope).exec();
                            }
                        }).exec();
                }
            } else {
                const auto accountId = message["accountId"].value<QString>();
                const auto type = message["type"].value<QString>();
                SyncScope scope;
                if (!accountId.isEmpty()) {
                    //FIXME this should work with either string or bytearray, but is apparently very picky
                    scope.resourceFilter<SinkResource::Account>(accountId.toLatin1());
                }
                scope.setType(type.toUtf8());
                SinkLog() << "Synchronizing... AccountId: " << accountId << " Type: " << scope.type();
                Store::synchronize(scope).exec();
            }
        }
        if (id == "abortSynchronization"/*Kube::Messages::abortSynchronization*/) {
            const auto accountId = message["accountId"].value<QString>();
            SyncScope scope;
            if (!accountId.isEmpty()) {
                //FIXME this should work with either string or bytearray, but is apparently very picky
                scope.resourceFilter<SinkResource::Account>(accountId.toLatin1());
            }
            Store::abortSynchronization(scope).exec();
        }
        if (id == "sendOutbox"/*Kube::Messages::synchronize*/) {
            Query query;
            query.containsFilter<SinkResource::Capabilities>(ResourceCapabilities::Mail::transport);
            auto job = Store::fetchAll<SinkResource>(query)
                .each([=](const SinkResource::Ptr &resource) -> KAsync::Job<void> {
                    return Store::synchronize(SyncScope{}.resourceFilter(resource->identifier()));
                });
            job.exec();
        }
        if (id == "markAsRead"/*Kube::Messages::synchronize*/) {
            if (auto mail = message["mail"].value<ApplicationDomain::Mail::Ptr>()) {
                mail->setUnread(false);
                Store::modify(*mail).exec();
            }
        }
        if (id == "markAsUnread"/*Kube::Messages::synchronize*/) {
            if (auto mail = message["mail"].value<ApplicationDomain::Mail::Ptr>()) {
                mail->setUnread(true);
                Store::modify(*mail).exec();
            }
        }
        if (id == "setImportant"/*Kube::Messages::synchronize*/) {
            if (auto mail = message["mail"].value<ApplicationDomain::Mail::Ptr>()) {
                mail->setImportant(message["important"].toBool());
                Store::modify(*mail).exec();
            }
        }
        if (id == "moveToTrash"/*Kube::Messages::synchronize*/) {
            if (auto mail = message["mail"].value<ApplicationDomain::Mail::Ptr>()) {
                mail->setTrash(true);
                Store::modify(*mail).exec();
            }
        }
        if (id == "restoreFromTrash") {
            if (auto mail = message["mail"].value<ApplicationDomain::Mail::Ptr>()) {
                mail->setTrash(false);
                Store::modify(*mail).exec();
            }
        }
        if (id == "moveToDrafts"/*Kube::Messages::synchronize*/) {
            if (auto mail = message["mail"].value<ApplicationDomain::Mail::Ptr>()) {
                mail->setDraft(true);
                Store::modify(*mail).exec();
            }
        }
        if (id == "moveToFolder"/*Kube::Messages::synchronize*/) {
            if (auto mail = message["mail"].value<ApplicationDomain::Mail::Ptr>()) {
                auto folder = message["folder"].value<ApplicationDomain::Folder::Ptr>();
                mail->setFolder(*folder);
                Store::modify(*mail).exec();
            }
        }
        if (id == "unlockKeyring") {
            auto accountId = message["accountId"].value<QByteArray>();
            Kube::AccountKeyring{accountId}.load();
        }
    }

};

class SinkNotifier {
public:

    static QVariantList toVariantList(const QByteArrayList &list)
    {
        QVariantList entities;
        for(const auto &entity : list) {
            entities << entity;
        }
        return entities;
    }

    static void sendErrorNotification(const Sink::Notification &notification)
    {
        QVariantMap message{
            {"type", "error"},
            {"resource", QString{notification.resource}},
            {"details", notification.message}
        };

        switch(notification.code) {
            case Sink::ApplicationDomain::ConnectionError:
                message["message"] = QObject::tr("Failed to connect to server.");
                message["subtype"] = "connectionError";
                break;
            case Sink::ApplicationDomain::NoServerError:
                message["message"] = QObject::tr("Host not found.");
                message["subtype"] = "hostNotFoundError";
                break;
            case Sink::ApplicationDomain::LoginError:
                message["message"] = QObject::tr("Failed to login.");
                message["subtype"] = "loginError";
                break;
            case Sink::ApplicationDomain::ConfigurationError:
                message["message"] = QObject::tr("Configuration error.");
                break;
            case Sink::ApplicationDomain::ConnectionLostError:
                //Ignore connection lost errors. We don't need them in the log view.
                return;
            case Sink::ApplicationDomain::MissingCredentialsError:
                message["message"] = QObject::tr("No credentials available.");
                break;
            default:
                //Ignore unknown errors, they are not going to help.
                return;
        }
        Fabric::Fabric{}.postMessage("errorNotification", message);
    }

    static void sendProgressNotification(int progress, int total, const QList<QByteArray> &entities, const QByteArray &resourceId)
    {
        QVariantMap message{
            {"type", "progress"},
            {"progress", progress},
            {"total", total},
            {"resourceId", resourceId}
        };

        if (!entities.isEmpty()) {
            message["folderId"] = entities.first();
        }
        Fabric::Fabric{}.postMessage("progressNotification", message);
    }

    SinkNotifier()
        : mNotifier{Sink::Query{Sink::Query::LiveQuery}}
    {
        mNotifier.registerHandler([] (const Sink::Notification &notification) {
            SinkLog() << "Received notification: " << notification;

            if (notification.type == Sink::Notification::Warning) {
                if (notification.code == Sink::ApplicationDomain::TransmissionError) {
                    Fabric::Fabric{}.postMessage("notification", {
                        {"type", "warning"},
                        {"message", QObject::tr("Failed to send message.")},
                        {"subtype", "transmissionError"},
                        {"entities", toVariantList(notification.entities)},
                        {"resource", QString{notification.resource}}
                    });
                }
            } else if (notification.type == Sink::Notification::Status) {
                return;
            } else if (notification.type == Sink::Notification::Error) {
                sendErrorNotification(notification);
            } else if (notification.type == Sink::Notification::Info) {
                if (notification.code == Sink::ApplicationDomain::TransmissionSuccess) {
                    Fabric::Fabric{}.postMessage("notification", {
                        {"type", "info"},
                        {"message", QObject::tr("A message has been sent.")},
                        {"subtype", "messageSent"},
                        {"entities", toVariantList(notification.entities)},
                        {"resource", QString{notification.resource}}
                    });
                } else if (notification.code == Sink::ApplicationDomain::NewContentAvailable) {
                    if (!notification.entities.isEmpty()) {
                        Fabric::Fabric{}.postMessage("notification", {
                            {"type", "info"},
                            {"folderId", notification.entities.first()},
                        });
                    }
                } else if (notification.code == Sink::ApplicationDomain::SyncInProgress) {
                    sendProgressNotification(0, 1, notification.entities, notification.resource);
                }
            } else if (notification.type == Sink::Notification::Progress) {
                sendProgressNotification(notification.progress, notification.total, notification.entities, notification.resource);
            }
        });

    }

    Sink::Notifier mNotifier;
};

class SinkFabric::Private
{
    SinkNotifier notifier;
    SinkListener listener;
};

SinkFabric::SinkFabric()
    : QObject(),
    d(new SinkFabric::Private)
{
}

SinkFabric::~SinkFabric()
{
    delete d;
}
SinkFabric &SinkFabric::instance()
{
    static SinkFabric instance;
    return instance;
}
