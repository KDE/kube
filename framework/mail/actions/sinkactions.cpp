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
#include "sinkactions.h"

#include <actions/context.h>

#include <sinkcommon/clientapi.h>

using namespace Kube;

ActionHandlerHelper::ActionHandlerHelper(const QByteArray &actionId, const std::function<bool(Context*)> &isReady, const std::function<void(Context*)> &handler)
    : ActionHandler(nullptr),
    isReadyFunction(isReady),
    handlerFunction(handler)
{
    setActionId(actionId);
}

bool ActionHandlerHelper::isActionReady(Context *context)
{
    return isReadyFunction(context);
}

void ActionHandlerHelper::execute(Context *context)
{
    handlerFunction(context);
}

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
        return context->property("folder").isValid();
    },
    [](Context *context) {
        auto folder = context->property("folder").value<Sink::ApplicationDomain::Folder::Ptr>();
        if (!folder) {
            qWarning() << "Failed to get the folder: " << context->property("folder");
            return;
        }
        Sink::Store::synchronize(Sink::Query::ResourceFilter(folder->resourceInstanceIdentifier())).exec();
    }
);
