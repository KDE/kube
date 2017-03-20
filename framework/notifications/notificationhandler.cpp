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

#include "notificationhandler.h"

#include <QDebug>

#include <sink/notifier.h>
#include <sink/notification.h>
#include <sink/query.h>

using namespace Kube;

NotificationHandler::NotificationHandler(QObject *parent)
    : QObject(parent)
{
    Sink::Query query{Sink::Query::LiveQuery};
    mNotifier.reset(new Sink::Notifier{query});
    mNotifier->registerHandler([this] (const Sink::Notification &notification) {
        notify(notification);
    });

}

void NotificationHandler::notify(const Sink::Notification &notification)
{
    Notification n;
    qWarning() << "Received notification: " << notification;
    if (notification.type == Sink::Notification::Warning) {
        n.mType = Notification::Warning;
        if (notification.code == Sink::ApplicationDomain::TransmissionError) {
            n.mMessage = "Failed to send message.";
        } else {
            return;
        }
    } else if (notification.type == Sink::Notification::Status) {
        if (notification.code == Sink::ApplicationDomain::ErrorStatus) {
            //A resource entered error status
            n.mType = Notification::Warning;
            n.mMessage = "A resource experienced an error.";
        } else {
            return;
        }
    } else if (notification.type == Sink::Notification::Info) {
        n.mType = Notification::Info;
        if (notification.code == Sink::ApplicationDomain::TransmissionSuccess) {
            n.mMessage = "A message has been sent.";
        } else {
            return;
        }
    } else {
        return;
    }
    //The base implementation to call the handler in QML
    QMetaObject::invokeMethod(this, "handler", Q_ARG(QVariant, QVariant::fromValue(&n)));
}

