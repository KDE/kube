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

class FolderContext : public Kube::ContextWrapper {
    using Kube::ContextWrapper::ContextWrapper;
    KUBE_CONTEXTWRAPPER_PROPERTY(Sink::ApplicationDomain::Folder::Ptr, Folder, folder)
};

static ActionHandlerHelper synchronizeHandler("org.kde.kube.actions.synchronize",
    [](Context *context) -> bool {
        return true;
    },
    [](Context *context_) {
        auto context = FolderContext{*context_};
        if (auto folder = context.getFolder()) {
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

static ActionHandlerHelper sendOutboxHandler("org.kde.kube.actions.sendOutbox",
    [](Context *context) -> bool {
        return true;
    },
    ActionHandlerHelper::JobHandler{[](Context *context) -> KAsync::Job<void> {
        using namespace Sink::ApplicationDomain;
        Query query;
        query.containsFilter<SinkResource::Capabilities>(ResourceCapabilities::Mail::transport);
        return Store::fetchAll<SinkResource>(query)
            .each([=](const SinkResource::Ptr &resource) -> KAsync::Job<void> {
                return Store::synchronize(SyncScope{}.resourceFilter(resource->identifier()));
            });
    }}
);

