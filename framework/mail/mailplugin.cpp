#include "mailplugin.h"

#include "maillistcontroller.h"
#include "maillistmodel.h"
#include "singlemailcontroller.h"
#include "folderlistcontroller.h"
#include "folderlistmodel.h"

#include <QAbstractItemModel>
#include <QtQml>

void MailPlugin::registerTypes (const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.akonadi2.mail"));

    qmlRegisterType<FolderListModel>();
    qmlRegisterType<FolderListController>(uri, 1, 0, "FolderList");

    qmlRegisterType<MailListModel>();
    qmlRegisterType<MailListController>(uri, 1, 0, "MailList");

    qmlRegisterType<SingleMailController>(uri, 1, 0, "SingleMail");
}
