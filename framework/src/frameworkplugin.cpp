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
#include "domain/mime/messageparser.h"
#include "domain/retriever.h"
#include "domain/outboxmodel.h"
#include "domain/mouseproxy.h"
#include "domain/contactcontroller.h"
#include "domain/peoplemodel.h"
#include "domain/textdocumenthandler.h"
#include "accounts/accountsmodel.h"
#include "accounts/accountfactory.h"
#include "settings/settings.h"
#include "fabric.h"
#include "kubeimage.h"
#include "clipboardproxy.h"
#include "webengineprofile.h"
#include "startupcheck.h"
#include "keyring.h"
#include "controller.h"

#include <QtQml>

static QObject *fabric_singletontype_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return new Kube::Fabric::Fabric;
}

static QObject *webengineprofile_singletontype_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return new WebEngineProfile;
}

static QObject *keyring_singletontype_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    auto instance = Kube::Keyring::instance();
    QQmlEngine::setObjectOwnership(instance, QQmlEngine::CppOwnership);
    return instance;
}

void FrameworkPlugin::registerTypes (const char *uri)
{
    qmlRegisterType<FolderListModel>(uri, 1, 0, "FolderListModel");
    qmlRegisterType<MailListModel>(uri, 1, 0, "MailListModel");
    qmlRegisterType<ComposerController>(uri, 1, 0, "ComposerController");
    qmlRegisterType<Kube::ControllerAction>(uri, 1, 0, "ControllerAction");
    qmlRegisterType<MessageParser>(uri, 1, 0, "MessageParser");
    qmlRegisterType<Retriever>(uri, 1, 0, "Retriever");
    qmlRegisterType<OutboxModel>(uri, 1, 0, "OutboxModel");
    qmlRegisterType<MouseProxy>(uri, 1, 0, "MouseProxy");
    qmlRegisterType<ContactController>(uri, 1, 0,"ContactController");
    qmlRegisterType<PeopleModel>(uri, 1, 0,"PeopleModel");
    qmlRegisterType<TextDocumentHandler>(uri, 1, 0, "TextDocumentHandler");

    qmlRegisterType<AccountFactory>(uri, 1, 0, "AccountFactory");
    qmlRegisterType<AccountsModel>(uri, 1, 0, "AccountsModel");

    qmlRegisterType<Kube::Settings>(uri, 1, 0, "Settings");

    qmlRegisterType<Kube::Fabric::Listener>(uri, 1, 0, "Listener");
    qmlRegisterSingletonType<Kube::Fabric::Fabric>(uri, 1, 0, "Fabric", fabric_singletontype_provider);

    qmlRegisterType<KubeImage>(uri, 1, 0, "KubeImage");
    qmlRegisterType<ClipboardProxy>(uri, 1, 0, "Clipboard");
    qmlRegisterType<StartupCheck>(uri, 1, 0, "StartupCheck");
    qmlRegisterSingletonType<WebEngineProfile>(uri, 1, 0, "WebEngineProfile", webengineprofile_singletontype_provider);
    qmlRegisterSingletonType<Kube::Keyring>(uri, 1, 0, "Keyring", keyring_singletontype_provider);
}
