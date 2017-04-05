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

