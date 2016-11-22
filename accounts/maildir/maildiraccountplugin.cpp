#include "maildiraccountplugin.h"

#include "maildirsettings.h"
#include "maildircontroller.h"

#include <QtQml>

void MaildirAccountPlugin::registerTypes (const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kube.accounts.maildir"));
    qmlRegisterType<MaildirSettings>(uri, 1, 0, "MaildirSettings");
    qmlRegisterType<MaildirController>(uri, 1, 0, "MaildirController");
}
