#include "actionplugin.h"

#include "action.h"
#include "context.h"
#include "actionhandler.h"
#include "actionresult.h"

#include <QtQml>

void KubePlugin::registerTypes (const char *uri)
{
    qmlRegisterType<Kube::Context>(uri, 1, 0, "Context");
    qmlRegisterType<Kube::Action>(uri, 1, 0, "Action");
    qmlRegisterType<Kube::ActionHandler>(uri, 1, 0, "ActionHandler");
    qmlRegisterType<Kube::ActionResult>(uri, 1, 0, "ActionResult");
}
