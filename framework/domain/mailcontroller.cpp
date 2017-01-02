/*
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
#include "mailcontroller.h"

#include <sink/store.h>
#include <sink/log.h>

SINK_DEBUG_AREA("mailcontroller");

MailController::MailController()
    : Kube::Controller(),
    action_markAsRead{new Kube::ControllerAction},
    action_moveToTrash{new Kube::ControllerAction},
    action_remove{new Kube::ControllerAction}
{
    QObject::connect(markAsReadAction(), &Kube::ControllerAction::triggered, this, &MailController::markAsRead);
    QObject::connect(moveToTrashAction(), &Kube::ControllerAction::triggered, this, &MailController::moveToTrash);
    QObject::connect(removeAction(), &Kube::ControllerAction::triggered, this, &MailController::remove);

    QObject::connect(this, &MailController::mailChanged, &MailController::updateActions);
    updateActions();
}

void MailController::updateActions()
{
    if (auto mail = getMail()) {
        action_moveToTrash->setEnabled(!mail->getTrash());
    }
}

void MailController::markAsRead()
{
    using namespace Sink;
    using namespace Sink::ApplicationDomain;
    auto mail = getMail();
    mail->setUnread(false);
    SinkLog() << "Mark as read " << mail->identifier();
    run(Store::modify(*mail));
}

void MailController::moveToTrash()
{
    using namespace Sink;
    using namespace Sink::ApplicationDomain;
    auto mail = getMail();
    mail->setTrash(true);
    SinkLog() << "Move to trash " << mail->identifier();
    run(Store::modify(*mail));
}

void MailController::remove()
{
    using namespace Sink;
    using namespace Sink::ApplicationDomain;
    auto mail = getMail();
    mail->setTrash(true);
    SinkLog() << "Remove " << mail->identifier();
    run(Store::remove(*mail));
}

