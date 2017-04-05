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
#pragma once

#include <QObject>
#include <QMultiMap>
#include <functional>
#include <KAsync/Async>

#include "actionresult.h"
#include "context.h"

namespace Kube {

class ActionHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QByteArray actionId READ actionId WRITE setActionId)

public:
    ActionHandler(QObject *parent = 0);
    virtual ~ActionHandler();

    virtual bool isActionReady(Context *context);

    virtual ActionResult execute(Context *context);
 
    void setActionId(const QByteArray &);
    QByteArray actionId() const;

    void setRequiredProperties(const QSet<QByteArray> &requiredProperties);
    QSet<QByteArray> requiredProperties() const;

private:
    QByteArray mActionId;
    QSet<QByteArray> mRequiredProperties;
};

template <typename ContextType>
class ActionHandlerBase : public ActionHandler
{
public:
    ActionHandlerBase(const QByteArray &actionId)
        : ActionHandler{}
    {
        setActionId(actionId);
    }

    bool isActionReady(Context *c) Q_DECL_OVERRIDE
    {
        auto wrapper = ContextType{*c};
        return isActionReady(wrapper);
    }

    ActionResult execute(Context *c) Q_DECL_OVERRIDE
    {
        ActionResult result;
        auto wrapper = ContextType{*c};
        execute(wrapper)
        .template then([=](const KAsync::Error &error) {
            auto modifyableResult = result;
            if (error) {
                qWarning() << "Job failed: " << error.errorCode << error.errorMessage;
                modifyableResult.setError(1);
            }
            modifyableResult.setDone();
        }).exec();
        return result;
    }
protected:

    virtual bool isActionReady(ContextType &) { return true; }
    virtual KAsync::Job<void> execute(ContextType &) = 0;
};

class ActionHandlerHelper : public ActionHandler
{
public:
    typedef std::function<bool(Context *)> IsReadyFunction;
    typedef std::function<void(Context *)> Handler;
    typedef std::function<KAsync::Job<void>(Context *)> JobHandler;

    ActionHandlerHelper(const Handler &);
    ActionHandlerHelper(const IsReadyFunction &, const Handler &);
    ActionHandlerHelper(const QByteArray &actionId, const IsReadyFunction &, const Handler &);
    ActionHandlerHelper(const QByteArray &actionId, const IsReadyFunction &, const JobHandler &);

    bool isActionReady(Context *) Q_DECL_OVERRIDE;
    ActionResult execute(Context *) Q_DECL_OVERRIDE;
private:
    const IsReadyFunction isReadyFunction;
    const Handler handlerFunction;
    const JobHandler jobHandlerFunction;
};

}
