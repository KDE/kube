#include "mailplugin.h"

#include "maillistmodel.h"
#include "folderlistmodel.h"

#include <QtQml>

void MailPlugin::registerTypes (const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.kube.mail"));

    qmlRegisterType<FolderListModel>(uri, 1, 0, "FolderListModel");
    qmlRegisterType<MailListModel>(uri, 1, 0, "MailListModel");
}
