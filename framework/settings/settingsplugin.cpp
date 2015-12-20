#include "settingsplugin.h"

#include "resourcescontroller.h"
#include "resourcelistmodel.h"
#include "maildir_resource.h"

#include <QtQml>

void SettingsPlugin::registerTypes (const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.akonadi2.settings"));

    qmlRegisterType<ResourceListModel>();
    qmlRegisterType<ResourcesController>(uri, 1, 0, "Resources");
    qmlRegisterType<MaildirResouceController>(uri, 1, 0, "Maildir");
}
