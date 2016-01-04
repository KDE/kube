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
#include "action.h"

#include <QDebug>
#include <QEvent>
#include <QDynamicPropertyChangeEvent>
#include <QMetaObject>
#include <QMetaProperty>

#include "actionbroker.h"
#include "context.h"

using namespace Kube;

Action::Action(QObject *parent)
    : QObject(parent),
    mContext(nullptr)
{
}

void Action::setContext(Context *context)
{
    //Get notified when any property changes
    for (int i = 0; i < context->metaObject()->propertyCount(); i++) {
        auto property = context->metaObject()->property(i) ;
        qWarning() << "Property " << property.name() << property.hasNotifySignal() << property.notifySignal().name();
        if (QString(property.name()) != "objectName") {
            //We do what SIGNAL does to connect to the changed signal automatically
            QObject::connect(context, "2"+property.notifySignal().name()+"()", this, SLOT(contextChanged()));
        }
    }
    mContext = context;
    mContext->installEventFilter(this);
    emit readyChanged();
}

void Action::contextChanged()
{
    emit readyChanged();
}

Context *Action::context() const
{
    return mContext;
}

void Action::setActionId(const QByteArray &actionId)
{
    mActionId = actionId;
    emit readyChanged();
}

QByteArray Action::actionId() const
{
    return mActionId;
}

bool Action::ready() const
{
    return ActionBroker::instance().isActionReady(mActionId, mContext);
}

void Action::execute()
{
    ActionBroker::instance().executeAction(mActionId, mContext);
}

