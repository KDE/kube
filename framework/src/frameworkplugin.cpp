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
#include "domain/mime/htmlutils.h"
#include "domain/perioddayeventmodel.h"
#include "domain/multidayeventmodel.h"
#include "domain/eventoccurrencemodel.h"
#include "domain/todomodel.h"
#include "domain/composercontroller.h"
#include "domain/mime/messageparser.h"
#include "domain/retriever.h"
#include "domain/outboxmodel.h"
#include "domain/mouseproxy.h"
#include "domain/contactcontroller.h"
#include "domain/eventcontroller.h"
#include "domain/invitationcontroller.h"
#include "domain/todocontroller.h"
#include "domain/peoplemodel.h"
#include "domain/textdocumenthandler.h"
#include "domain/settings/accountsettings.h"
#include "accounts/accountsmodel.h"
#include "accounts/accountfactory.h"
#include "settings/settings.h"
#include "fabric.h"
#include "kubeimage.h"
#include "clipboardproxy.h"
#include "startupcheck.h"
#include "keyring.h"
#include "controller.h"
#include "domainobjectcontroller.h"
#include "extensionmodel.h"
#include "viewhighlighter.h"
#include "file.h"
#include "logmodel.h"
#include "inboundmodel.h"
#include "entitymodel.h"
#include "entitycontroller.h"
#include "qquicktreemodeladaptor.h"

#include <QtQml>
#include <QQuickImageProvider>
#include <QIcon>
#include <QApplication>

class KubeImageProvider : public QQuickImageProvider
{
public:
    KubeImageProvider()
        : QQuickImageProvider(QQuickImageProvider::Pixmap)
    {
    }

    static QSize selectSize(const QSize &requestedSize, const QList<QSize> &availableSizes)
    {
        auto expectedSize = requestedSize;
        //Get the largest size that is still smaller or equal than requested
        //Except if we only have larger sizes, then just pick the closest one
        bool first = true;
        for (const auto &s : availableSizes) {
            if (first && s.width() > requestedSize.width()) {
                return s;
            }
            first = false;
            if (s.width() <= requestedSize.width()) {
                expectedSize = s;
            }
        }
        return expectedSize;
    }

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) Q_DECL_OVERRIDE
    {
        //The platform theme plugin can overwrite our setting again once it gets loaded,
        //so we check on every icon load request...
        if (QIcon::themeName() != "kube") {
            QIcon::setThemeName("kube");
        }
        const auto icon = QIcon::fromTheme(id);
        static auto devicePixelRatio = static_cast<QApplication*>(QApplication::instance())->devicePixelRatio();
        //availableSizes() does not take the devicePixelRatio into account, so if we divide the request by it first,
        //we will end up with the correct size after multiplying it later.
        const auto expectedSize = selectSize(requestedSize / devicePixelRatio, icon.availableSizes());
        auto pixmap = icon.pixmap(expectedSize * devicePixelRatio);
        pixmap.setDevicePixelRatio(devicePixelRatio);
        if (size) {
            *size = pixmap.size();
        }
        return pixmap;
    }
};


static QObject *fabric_singletontype_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return new Kube::Fabric::Fabric;
}

static QObject *htmlutils_singletontype_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return new HtmlUtils::HtmlUtils;
}

static QObject *keyring_singletontype_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    auto instance = Kube::Keyring::instance();
    QQmlEngine::setObjectOwnership(instance, QQmlEngine::CppOwnership);
    return instance;
}

static QString findFile(const QString file, const QStringList importPathList)
{
    for (const auto &path : importPathList) {
        const QString f = path + file;
        if (QFileInfo::exists(f)) {
            return f;
        }
    }
    return {};
}

void FrameworkPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    Q_UNUSED(uri);
    engine->addImageProvider(QLatin1String("kube"), new KubeImageProvider);

    QString kubeIcons = QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("kube-icons.rcc"));
    //For windows
    if (kubeIcons.isEmpty()) {
        const auto locations = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation) + QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
        kubeIcons = findFile(QStringLiteral("/kube/kube-icons.rcc"), locations);
    }
    //For osx
    if (kubeIcons.isEmpty()) {
        //On Mac OS we want to include Contents/Resources/ in the bundle, and that path is in AppDataLocations.
        QStringList iconSearchPaths;
        for (const auto &p : QStandardPaths::standardLocations(QStandardPaths::AppDataLocation)) {
            auto iconPath = p;
            //I'm getting broken paths reported from standardLocations
            if (iconPath.contains("kube.appContents")) {
                iconPath.replace("kube.appContents", "kube.app/Contents");
            }
            if (iconPath.contains("kube-kolabnow.appContents")) {
                iconPath.replace("kube-kolabnow.appContents", "kube-kolabnow.app/Contents");
            }
            iconSearchPaths << iconPath;
        }
        kubeIcons = findFile(QStringLiteral("/kube/kube-icons.rcc"), iconSearchPaths);
    }

    if (!QResource::registerResource(kubeIcons, "/icons/kube")) {
        qWarning() << "Failed to register icon resource!" << kubeIcons;
        qWarning() << "Searched paths: " << QStandardPaths::standardLocations(QStandardPaths::AppDataLocation) + QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
        Q_ASSERT(false);
    } else {
        QIcon::setThemeSearchPaths(QStringList() << QStringLiteral(":/icons"));
        QIcon::setThemeName(QStringLiteral("kube"));
    }
}

void FrameworkPlugin::registerTypes (const char *uri)
{
    qmlRegisterType<FolderListModel>(uri, 1, 0, "FolderListModel");
    qmlRegisterType<MailListModel>(uri, 1, 0, "MailListModel");
    qmlRegisterType<PeriodDayEventModel>(uri, 1, 0, "PeriodDayEventModel");
    qmlRegisterType<MultiDayEventModel>(uri, 1, 0, "MultiDayEventModel");
    qmlRegisterType<EventOccurrenceModel>(uri, 1, 0, "EventOccurrenceModel");
    qmlRegisterType<EventController>(uri, 1, 0, "EventController");
    qmlRegisterType<InvitationController>(uri, 1, 0, "InvitationController");
    qmlRegisterType<TodoModel>(uri, 1, 0, "TodoModel");
    qmlRegisterType<TodoController>(uri, 1, 0, "TodoController");
    qmlRegisterType<ComposerController>(uri, 1, 0, "ComposerController");
    qmlRegisterUncreatableType<Kube::ListPropertyController>(uri, 1, 0, "ListPropertyController", "abstract");
    qmlRegisterUncreatableType<Selector>(uri, 1, 0, "Selector", "abstract");
    qmlRegisterUncreatableType<Completer>(uri, 1, 0, "Completer", "abstract");
    qmlRegisterType<Kube::ControllerAction>(uri, 1, 0, "ControllerAction");
    qmlRegisterType<MessageParser>(uri, 1, 0, "MessageParser");
    qmlRegisterType<Retriever>(uri, 1, 0, "Retriever");
    qmlRegisterType<OutboxModel>(uri, 1, 0, "OutboxModel");
    qmlRegisterType<MouseProxy>(uri, 1, 0, "MouseProxy");
    qmlRegisterType<ContactController>(uri, 1, 0,"ContactController");
    qmlRegisterType<PeopleModel>(uri, 1, 0,"PeopleModel");
    qmlRegisterType<TextDocumentHandler>(uri, 1, 0, "TextDocumentHandler");
    qmlRegisterType<LogModel>(uri, 1, 0, "LogModel");
    qmlRegisterType<InboundModel>(uri, 1, 0, "InboundModel");
    qmlRegisterType<EntityModel>(uri, 1, 0, "EntityModel");
    qmlRegisterType<EntityLoader>(uri, 1, 0, "EntityLoader");
    qmlRegisterType<EntityController>(uri, 1, 0, "EntityController");
    qmlRegisterType<CheckedEntities>(uri, 1, 0, "CheckedEntities");
    qmlRegisterType<CheckableEntityModel>(uri, 1, 0, "CheckableEntityModel");
    qmlRegisterType<QQuickTreeModelAdaptor1>(uri, 1, 0, "TreeModelAdaptor");
    qmlRegisterSingletonType<HtmlUtils::HtmlUtils>(uri, 1, 0, "HtmlUtils", htmlutils_singletontype_provider);

    qmlRegisterType<AccountFactory>(uri, 1, 0, "AccountFactory");
    qmlRegisterType<AccountsModel>(uri, 1, 0, "AccountsModel");
    qmlRegisterType<AccountSettings>(uri, 1, 0, "AccountSettings");
    qmlRegisterType<Kube::ExtensionModel>(uri, 1, 0, "ExtensionModel");
    qmlRegisterType<Kube::File>(uri, 1, 0, "File");

    qmlRegisterType<Kube::Settings>(uri, 1, 0, "Settings");

    qmlRegisterType<Kube::Fabric::Listener>(uri, 1, 0, "Listener");
    qmlRegisterType<Kube::DomainObjectController>(uri, 1, 0, "DomainObjectController");
    qmlRegisterSingletonType<Kube::Fabric::Fabric>(uri, 1, 0, "Fabric", fabric_singletontype_provider);

    qmlRegisterType<KubeImage>(uri, 1, 0, "KubeImage");
    qmlRegisterType<ClipboardProxy>(uri, 1, 0, "Clipboard");
    qmlRegisterType<StartupCheck>(uri, 1, 0, "StartupCheck");
    qmlRegisterType<ViewHighlighter>(uri, 1, 0, "ViewHighlighter");
    qmlRegisterSingletonType<Kube::Keyring>(uri, 1, 0, "Keyring", keyring_singletontype_provider);
}
