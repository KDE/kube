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

#include "fabric.h"

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
            } else {
                auto accountId = message["accountId"].value<QString>();
                auto type = message["type"].value<QString>();
                SyncScope scope;
                if (!accountId.isEmpty()) {
                    //FIXME this should work with either string or bytearray, but is apparently very picky
                    scope.resourceFilter<SinkResource::Account>(accountId.toLatin1());
                }
                if (type == "contacts") {
                    scope.setType<ApplicationDomain::Contact>();
                } else if (type == "mail") {
                    scope.setType<ApplicationDomain::Mail>();
                } else if (type == "folder") {
                    scope.setType<ApplicationDomain::Folder>();
                } else {
                    //Only synchronize folders by default for now
                    scope.setType<ApplicationDomain::Folder>();
                }
                SinkLog() << "Synchronizing all. AccountId: " << accountId << " Type: " << scope.type();
                Store::synchronize(scope).exec();
            }
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
        if (id == "toggleImportant"/*Kube::Messages::synchronize*/) {
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

    }

};

class SinkNotifier {
public:
    SinkNotifier()
        : mNotifier{Sink::Query{Sink::Query::LiveQuery}}
    {
        mNotifier.registerHandler([this] (const Sink::Notification &notification) {
            Notification n;
            SinkLog() << "Received notification: " << notification;
            QVariantMap message;
            if (notification.type == Sink::Notification::Warning) {
                message["type"] = Notification::Warning;
                if (notification.code == Sink::ApplicationDomain::TransmissionError) {
                    message["message"] = QObject::tr("Failed to send message.");
                } else {
                    return;
                }
            } else if (notification.type == Sink::Notification::Status) {
                if (notification.code == Sink::ApplicationDomain::ErrorStatus) {
                    //A resource entered error status
                    message["type"] = Notification::Warning;
                    message["message"] = QObject::tr("A resource experienced an error.");
                } else {
                    return;
                }
            } else if (notification.type == Sink::Notification::Error) {
                message["type"] = Notification::Warning;
                message["resource"] = QString{notification.resource};
                switch(notification.code) {
                    case Sink::ApplicationDomain::ConnectionError:
                        message["message"] = QObject::tr("Failed to connect to server.");
                        break;
                    case Sink::ApplicationDomain::NoServerError:
                        message["message"] = QObject::tr("Host not found.");
                        break;
                    case Sink::ApplicationDomain::LoginError:
                        message["message"] = QObject::tr("Failed to login.");
                        break;
                    case Sink::ApplicationDomain::ConfigurationError:
                        message["message"] = QObject::tr("Configuration error.");
                        break;
                    case Sink::ApplicationDomain::ConnectionLostError:
                        message["message"] = QObject::tr("Connection lost.");
                        break;
                    default:
                        message["message"] = "An unknown error occurred: " + notification.message;
                }
                Fabric::Fabric{}.postMessage("errorNotification", message);
            } else if (notification.type == Sink::Notification::Info) {
                if (notification.code == Sink::ApplicationDomain::TransmissionSuccess) {
                    message["type"] = Notification::Info;
                    message["message"] = QObject::tr("A message has been sent.");
                } else {
                    return;
                }
            } else if (notification.type == Sink::Notification::Progress) {
                message["progress"] = notification.progress;
                message["total"] = notification.total;
                if (!notification.entities.isEmpty()) {
                    message["folderId"] = notification.entities.first();
                }
                message["resourceId"] = notification.resource;
                Fabric::Fabric{}.postMessage("progressNotification", message);
                return;
            } else {
                return;
            }
            Fabric::Fabric{}.postMessage("notification", message);

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
