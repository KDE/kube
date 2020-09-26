/*
    Copyright (c) 2020 Christian Mollekopf <mollekopf@kolabsys.com>

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
#include "dbusinterface.h"

#include <QString>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QDBusAbstractAdaptor>

QString service = "com.kubeproject.kube";
QString path = "/";
QString interface = "com.kubeproject.rpc";

class Adaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kubeproject.rpc")
public:
    Adaptor(QObject *obj) : QDBusAbstractAdaptor(obj) {}
signals:
    void activated(const QString &view);

public slots:
    Q_NOREPLY void activate(const QString &view) {
        emit activated(view);
    }
};

bool DBusInterface::registerService()
{
    auto bus = QDBusConnection::sessionBus();
    bool ret =  bus.registerService(service);
    if (ret) {
        auto adaptor = new Adaptor(this);
        connect(adaptor, &Adaptor::activated, this, &DBusInterface::activated);
        bus.registerObject(path, this);
    }
    return ret;
}

void DBusInterface::activate(const QString &view)
{
    QDBusInterface iface(service, path, interface, QDBusConnection::sessionBus());
    if (iface.isValid()) {
        iface.call("activate", view);
    }
}

#include "dbusinterface.moc"
