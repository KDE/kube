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

#include "fabric.h"
#include "sinkfabric.h"
#include "notificationfabric.h"

#include <QDebug>

namespace Kube {
namespace Fabric {

class Bus {
public:
    Bus() = default;
    ~Bus() = default;

    static Bus &instance()
    {
        static Bus bus;
        return bus;
    }

    void bringUpDeps()
    {
        if (!mDepsUp) {
            mDepsUp = true;
            SinkFabric::instance();
            NotificationFabric::instance();
        }
    }

    void registerListener(Listener *listener)
    {
        mListener << listener;
        bringUpDeps();
    }

    void unregisterListener(Listener *listener)
    {
        mListener.removeAll(listener);
    }

    void postMessage(const QString &id, const QVariantMap &message)
    {
        bringUpDeps();
        for (const auto &l : mListener) {
            l->notify(id, message);
        }
    }

private:
    QVector<Listener*> mListener;
    bool mDepsUp = false;
};

void Fabric::postMessage(const QString &id, const QVariantMap &msg)
{
    Bus::instance().postMessage(id, msg);
}

Listener::Listener(QObject *parent)
    : QObject(parent)
{
    Bus::instance().registerListener(this);
}

Listener::~Listener()
{
    Bus::instance().unregisterListener(this);
}

void Listener::notify(const QString &messageId, const QVariantMap &msg)
{
    if (messageId == mFilter) {
        emit messageReceived(msg);
    }
}

}
}
