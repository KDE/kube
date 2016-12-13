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

#include "actionbroker.h"

#include "context.h"
#include "actionhandler.h"

#include <QDebug>

using namespace Kube;

ActionBroker::ActionBroker(QObject *parent)
    : QObject(parent)
{

}

ActionBroker &ActionBroker::instance()
{
    static ActionBroker instance;
    return instance;
}

bool ActionBroker::isActionReady(const QByteArray &actionId, Context *context)
{
    if (!context) {
        return false;
    }

    for (const auto handler : mHandler.values(actionId)) {
        if (handler) {
            if (handler-> isActionReady(context)) {
                return true;
            }
        }
    }

    return false;
}

ActionResult ActionBroker::executeAction(const QByteArray &actionId, Context *context, const QList<QPointer<ActionHandler>> &preHandler, const QList<QPointer<ActionHandler>> &postHandler)
{
    ActionResult result;
    if (context) {
        for (const auto handler : preHandler) {
            handler->execute(context);
        }
        //TODO the main handler should only execute once the pre handler is done
        for (const auto handler : mHandler.values(actionId)) {
            if (handler) {
                result += handler->execute(context);
            }
        }
        //TODO the post handler should only execute once the main handler is done
        for (const auto handler : postHandler) {
            handler->execute(context);
        }
    } else {
        qWarning() << "Can't execute without context";
        result.setDone();
        result.setError(1);
    }
    return result;
}

void ActionBroker::registerHandler(const QByteArray &actionId, ActionHandler *handler)
{
    mHandler.insert(actionId, handler);
}
