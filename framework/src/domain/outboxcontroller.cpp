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
#include "outboxcontroller.h"

#include <sink/store.h>
#include <sink/log.h>

OutboxController::OutboxController()
    : Kube::Controller(),
    mSynchronizeOutboxAction{new Kube::ControllerAction{this, &OutboxController::sendOutbox}}
{
}

Kube::ControllerAction* OutboxController::sendOutboxAction() const
{
    return mSynchronizeOutboxAction.data();
}

void OutboxController::sendOutbox()
{
    using namespace Sink;
    using namespace Sink::ApplicationDomain;
    Query query;
    query.containsFilter<SinkResource::Capabilities>(ResourceCapabilities::Mail::transport);
    auto job = Store::fetchAll<SinkResource>(query)
        .each([=](const SinkResource::Ptr &resource) -> KAsync::Job<void> {
            return Store::synchronize(SyncScope{}.resourceFilter(resource->identifier()));
        });
    run(job);
}

