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
#include <Async/Async>

#include "actionresult.h"

namespace Kube {
class Context;

class ActionHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QByteArray actionId READ actionId WRITE setActionId)

public:
    ActionHandler(QObject *parent = 0);

    virtual bool isActionReady(Context *context);

    virtual ActionResult execute(Context *context);
 
    void setActionId(const QByteArray &);
    QByteArray actionId() const;

private:
    QByteArray mActionId;
};

class ActionHandlerHelper : public ActionHandler
{
    Q_OBJECT
public:
    typedef std::function<bool(Context*)> IsReadyFunction;
    typedef std::function<void(Context*)> Handler;
    typedef std::function<KAsync::Job<void>(Context*)> JobHandler;

    ActionHandlerHelper(const QByteArray &actionId, const IsReadyFunction &, const Handler &);
    ActionHandlerHelper(const QByteArray &actionId, const IsReadyFunction &, const JobHandler &);

    bool isActionReady(Context *context) Q_DECL_OVERRIDE;
    ActionResult execute(Context *context) Q_DECL_OVERRIDE;
private:
    const IsReadyFunction isReadyFunction;
    const Handler handlerFunction;
    const JobHandler jobHandlerFunction;
};

}
