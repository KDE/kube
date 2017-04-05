/*
    Copyright (c) 2016 Michael Bohlender <michael.bohlender@kdemail.net>
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

#include "frameworkplugin.h"

#include "domain/maillistmodel.h"
#include "domain/folderlistmodel.h"
#include "domain/composercontroller.h"
#include "domain/messageparser.h"
#include "domain/retriever.h"
#include "domain/outboxmodel.h"
#include "domain/outboxcontroller.h"
#include "domain/mailcontroller.h"
#include "domain/foldercontroller.h"
#include "domain/mouseproxy.h"
#include "domain/contactcontroller.h"
#include "domain/peoplemodel.h"
#include "accounts/accountsmodel.h"
#include "accounts/accountfactory.h"
#include "settings/settings.h"
#include "notifications/notificationhandler.h"
#include "actions/action.h"
#include "actions/context.h"
#include "actions/actionhandler.h"
#include "actions/actionresult.h"

#include <QtQml>

void FrameworkPlugin::registerTypes (const char *uri)
{
    qmlRegisterType<FolderListModel>(uri, 1, 0, "FolderListModel");
    qmlRegisterType<MailListModel>(uri, 1, 0, "MailListModel");
    qmlRegisterType<ComposerController>(uri, 1, 0, "ComposerController");
    qmlRegisterType<MessageParser>(uri, 1, 0, "MessageParser");
    qmlRegisterType<Retriever>(uri, 1, 0, "Retriever");
    qmlRegisterType<OutboxController>(uri, 1, 0, "OutboxController");
    qmlRegisterType<OutboxModel>(uri, 1, 0, "OutboxModel");
    qmlRegisterType<MailController>(uri, 1, 0, "MailController");
    qmlRegisterType<FolderController>(uri, 1, 0, "FolderController");
    qmlRegisterType<MouseProxy>(uri, 1, 0, "MouseProxy");
    qmlRegisterType<ContactController>(uri, 1, 0,"ContactController");
    qmlRegisterType<PeopleModel>(uri, 1, 0,"PeopleModel");

    qmlRegisterType<AccountFactory>(uri, 1, 0, "AccountFactory");
    qmlRegisterType<AccountsModel>(uri, 1, 0, "AccountsModel");

    qmlRegisterType<Kube::Settings>(uri, 1, 0, "Settings");
    qmlRegisterType<Kube::NotificationHandler>(uri, 1, 0, "NotificationHandler");
    qmlRegisterType<Kube::Notification>(uri, 1, 0, "Notification");

    qmlRegisterType<Kube::Context>(uri, 1, 0, "Context");
    qmlRegisterType<Kube::Action>(uri, 1, 0, "Action");
    qmlRegisterType<Kube::ActionHandler>(uri, 1, 0, "ActionHandler");
    qmlRegisterType<Kube::ActionResult>(uri, 1, 0, "ActionResult");
}
