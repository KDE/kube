/*
    Copyright (c) 2021 Christian Mollekopf <mollekopf@kolabsys.com>

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
#include "notificationfabric.h"

#include <QFile>
#include <QDBusInterface>
#include <QDBusReply>

#include "fabric.h"

#include <sink/log.h>

using namespace Kube;

class NotificationListener : public Kube::Fabric::Listener
{
    Q_OBJECT

    quint32 mNotificationId = 0;

public:
    NotificationListener()
        : Kube::Fabric::Listener()
    {
        QDBusConnection::sessionBus().connect(QString(), QString(), "org.freedesktop.Notifications",
                                            "NotificationClosed", this, SLOT(onNotificationClosed(quint32, quint32)));
        // QDBusConnection::sessionBus().connect(QString(), QString(), org::freedesktop::Notifications::staticInterfaceName(),
        //                                     "ActionInvoked", this, SLOT(onActionInvoked(quint32,QString)));
    }

    quint32 dbusNotify(const QString &appName, quint32 replacesId,
                        const QString & appIcon,
                        const QString & summary, const QString & body,
                        const QStringList & actions,
                        const QVariantMap & hints,
                        qint32 timeout)
    {
        QDBusInterface iface("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", QDBusConnection::sessionBus());
        if (iface.isValid()) {
            QDBusReply<quint32> id = iface.call("Notify", appName, replacesId, appIcon, summary, body, actions, hints, timeout);
            return id;
        }
        return 0;
    }

    void notify(const QString &id, const QVariantMap &message)
    {
        SinkWarning() << "Received message: " << id << message;
        if (id == "displayNotification"/*Kube::Messages::displayNotification*/) {
            mNotificationId = dbusNotify("Kube", mNotificationId, "", "Kube", message.value("message").toString(), {}, {}, -1);
        }
    }
private slots:

    void onNotificationClosed(quint32, quint32)
    {
        qWarning() << "Notification closed";

        const QString service = "com.kubeproject.kube";
        const QString path = "/";
        const QString interface = "com.kubeproject.rpc";

        QDBusInterface iface(service, path, interface, QDBusConnection::sessionBus());
        if (iface.isValid()) {
            iface.call("activate", "");
        }
    }

    // void onActionInvoked(quint32, QString)
    // {

    // }
};

class NotificationFabric::Private
{
    NotificationListener listener;
};

NotificationFabric::NotificationFabric()
    : QObject(),
    d(new NotificationFabric::Private)
{
}

NotificationFabric::~NotificationFabric()
{
    delete d;
}

NotificationFabric &NotificationFabric::instance()
{
    static NotificationFabric instance;
    return instance;
}

#include "notificationfabric.moc"
