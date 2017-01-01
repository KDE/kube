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
#include "foldercontroller.h"

#include <sink/store.h>
#include <sink/log.h>

SINK_DEBUG_AREA("foldercontroller");

FolderController::FolderController()
    : Kube::Controller(),
    action_synchronize{new Kube::ControllerAction}
{
    QObject::connect(synchronizeAction(), &Kube::ControllerAction::triggered, this, &FolderController::synchronize);
}

void FolderController::synchronize()
{
    using namespace Sink;
    using namespace Sink::ApplicationDomain;
    auto job = [&] {
        if (auto folder = getFolder()) {
            SinkLog() << "Synchronizing folder " << folder->resourceInstanceIdentifier() << folder->identifier();
            auto scope = SyncScope().resourceFilter(folder->resourceInstanceIdentifier()).filter<Mail::Folder>(QVariant::fromValue(folder->identifier()));
            scope.setType<ApplicationDomain::Mail>();
            return Store::synchronize(scope);
        } else {
            SinkLog() << "Synchronizing all";
            return Store::synchronize(SyncScope());
        }
    }();
    run(job);
}

