#include "maildiraccountplugin.h"

#include "maildirsettings.h"

#include <QtQml>

void MaildirAccountPlugin::registerTypes (const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.kube.accounts.maildir"));
    qmlRegisterType<MaildirSettings>(uri, 1, 0, "MaildirSettings");
}
