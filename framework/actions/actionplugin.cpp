#include "actionplugin.h"

#include "action.h"
#include "context.h"
#include "actionhandler.h"

#include <QtQml>

void KubePlugin::registerTypes (const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.kube.actions"));
    qmlRegisterType<Kube::Context>(uri, 1, 0, "Context");
    qmlRegisterType<Kube::Action>(uri, 1, 0, "Action");
    qmlRegisterType<Kube::ActionHandler>(uri, 1, 0, "ActionHandler");
}
