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
#include <actions/context.h>
#include <actions/actionhandler.h>

#include "../mailtransport.h"
#include <KMime/Message>
#include <QByteArray>
#include <QVariant>
#include <QDebug>

using namespace Kube;

static ActionHandlerHelper sendMailHandler("org.kde.kube.actions.sendmail",
    [](Context *context) -> bool {
        auto username = context->property("username").value<QByteArray>();
        auto password = context->property("password").value<QByteArray>();
        auto server = context->property("server").value<QByteArray>();
        auto message = context->property("message").value<KMime::Message::Ptr>();
        return !username.isEmpty() && !password.isEmpty() && !server.isEmpty() && message;
    },
    [](Context *context) {
        auto username = context->property("username").value<QByteArray>();
        auto password = context->property("password").value<QByteArray>();
        auto server = context->property("server").value<QByteArray>();
        //For ssl use "smtps://mainserver.example.net
        QByteArray cacert; // = "/path/to/certificate.pem";
        auto message = context->property("message").value<KMime::Message::Ptr>();
        qWarning() << "Sending a mail: ";
        MailTransport::sendMessage(message, server, username, password, cacert);
    }
);
