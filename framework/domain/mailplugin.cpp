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

#include "mailplugin.h"

#include "maillistmodel.h"
#include "folderlistmodel.h"
#include "composercontroller.h"
#include "messageparser.h"
#include "retriever.h"
#include "accountfactory.h"
#include "accountscontroller.h"
#include "accountsmodel.h"
#include "outboxmodel.h"

#include <QtQml>

void MailPlugin::registerTypes (const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kube.framework.domain"));

    qmlRegisterType<FolderListModel>(uri, 1, 0, "FolderListModel");
    qmlRegisterType<MailListModel>(uri, 1, 0, "MailListModel");
    qmlRegisterType<ComposerController>(uri, 1, 0, "ComposerController");
    qmlRegisterType<MessageParser>(uri, 1, 0, "MessageParser");
    qmlRegisterType<Retriever>(uri, 1, 0, "Retriever");
    qmlRegisterType<AccountFactory>(uri, 1, 0, "AccountFactory");
    qmlRegisterType<AccountsController>(uri, 1, 0, "AccountsController");
    qmlRegisterType<AccountsModel>(uri, 1, 0, "AccountsModel");
    qmlRegisterType<OutboxModel>(uri, 1, 0, "OutboxModel");
}
