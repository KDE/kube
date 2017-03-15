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
#include <functional>
#include <sink/notifier.h>

namespace Sink {
    class Notification;
}

namespace Kube {

class Notification : public QObject
{
    Q_OBJECT

public:
    enum Type {
        Info,
        Warning
    };
    Q_ENUMS(Type)

    Q_PROPERTY(Type type MEMBER mType CONSTANT)
    Q_PROPERTY(QString message MEMBER mMessage CONSTANT)

    Notification() = default;
    ~Notification() = default;

    Type mType;
    QString mMessage;
};

/**
 * A notification handler.
 *
 * Can beinstantiated from QML like so:
 * NotificationHandler {
 *   function handler(notification) {
 *      ...do something
 *   }
 * }
 *
 * The handler will listen for all notifications by default.
 */
class NotificationHandler : public QObject
{
    Q_OBJECT

public:
    NotificationHandler(QObject *parent = 0);

    virtual void notify(const Sink::Notification &notification);

private:
    QScopedPointer<Sink::Notifier> mNotifier;
};

}

Q_DECLARE_METATYPE(Kube::Notification*);
