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
#include <QVariant>
#include <QVector>

namespace Kube {
/**
 * The Fabric is a publish/subscribe message bus to interconnect components in kube.
 *
 * It provides a background mesh ("the fabric"), that interconnects various parts of kube.
 * This is useful as it allows us to decouple functionality from the UI components, and keeps us from writing unnecessary API
 * for visual components to pass through all necessary information for interaction.
 */
namespace Fabric {

class Fabric : public QObject {
    Q_OBJECT
public:
    Q_INVOKABLE void postMessage(const QString &id, const QVariantMap &);
};

/**
 * A message handler.
 *
 * Can beinstantiated from QML like so:
 * Listener {
 *   function onMessageReceived(msg) {
 *      ...do something
 *   }
 * }
 */
class Listener : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString filter MEMBER mFilter)

public:
    Listener(QObject *parent = 0);
    virtual ~Listener();

    virtual void notify(const QString &id, const QVariantMap &notification);

signals:
    void messageReceived(const QVariantMap &message);

private:
    QString mFilter;
};

}
}
