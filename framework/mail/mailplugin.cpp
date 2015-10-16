#include "mailplugin.h"

#include "maillistcontroller.h"
#include "maillistmodel.h"

#include <QtQml>

void MailPlugin::registerTypes (const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.akonadi2.mail"));
    qmlRegisterType<MailListController>(uri, 1, 0, "MailList");
    qmlRegisterType<MailListModel>(uri, 1, 0, "MailListModel");
}