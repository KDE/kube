#include "mailplugin.h"

#include "maillistcontroller.h"
#include "maillistmodel.h"
#include <QAbstractItemModel>

#include <QtQml>

void MailPlugin::registerTypes (const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.akonadi2.mail"));
    qmlRegisterType<QAbstractItemModel>();
    qmlRegisterType<MailListController>(uri, 1, 0, "MailList");
}
