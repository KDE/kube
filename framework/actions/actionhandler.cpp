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

#include "actionhandler.h"

#include "context.h"
#include "actionbroker.h"
#include <QDebug>

using namespace Kube;

ActionHandler::ActionHandler(QObject *parent)
    : QObject(parent)
{

}

bool ActionHandler::isActionReady(Context *context)
{
    if (context) {
        QVariant returnedValue;
        QMetaObject::invokeMethod(this, "isReady",
                                Q_RETURN_ARG(QVariant, returnedValue),
                                Q_ARG(QVariant, QVariant::fromValue(context)));
        return returnedValue.toBool();
    } else {
        qWarning() << "The handler didn't get a context";
    }
    return false;
}

ActionResult ActionHandler::execute(Context *context)
{
    ActionResult result;
    QVariant returnedValue;
    qWarning() << "Executing the handler";
    if (context) {
        //The base implementation to call the handler in QML
        QMetaObject::invokeMethod(this, "handler",
                                Q_RETURN_ARG(QVariant, returnedValue),
                                Q_ARG(QVariant, QVariant::fromValue(context)));
        //TODO: support async handlers in QML
        result.setDone();
    } else {
        qWarning() << "The handler didn't get a context";
        result.setDone();
        result.setError(1);
    }
    return result;
}

void ActionHandler::setActionId(const QByteArray &actionId)
{
    mActionId = actionId;
    ActionBroker::instance().registerHandler(actionId, this);
}

QByteArray ActionHandler::actionId() const
{
    return mActionId;
}



ActionHandlerHelper::ActionHandlerHelper(const QByteArray &actionId, const IsReadyFunction &isReady, const Handler &handler)
    : ActionHandler(nullptr),
    isReadyFunction(isReady),
    handlerFunction(handler)
{
    setActionId(actionId);
}

ActionHandlerHelper::ActionHandlerHelper(const QByteArray &actionId, const IsReadyFunction &isReady, const JobHandler &handler)
    : ActionHandler(nullptr),
    isReadyFunction(isReady),
    jobHandlerFunction(handler)
{
    setActionId(actionId);
}

bool ActionHandlerHelper::isActionReady(Context *context)
{
    return isReadyFunction(context);
}

ActionResult ActionHandlerHelper::execute(Context *context)
{
    ActionResult result;
    if (handlerFunction) {
        handlerFunction(context);
        result.setDone();
    } else {
        jobHandlerFunction(context).then<void>([=]() {
            auto modifyableResult = result;
            modifyableResult.setDone();
        },
        [=](int errorCode, const QString &string) {
            qWarning() << "Job failed: " << errorCode << string;
            auto modifyableResult = result;
            modifyableResult.setError(1);
            modifyableResult.setDone();
        }).exec();
    }
    return result;
}
