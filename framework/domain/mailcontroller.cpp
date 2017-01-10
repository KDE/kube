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

using namespace Sink;
using namespace Sink::ApplicationDomain;

MailController::MailController()
    : Kube::Controller(),
    action_markAsRead{new Kube::ControllerAction{this, &MailController::markAsRead}},
    action_markAsUnread{new Kube::ControllerAction{this, &MailController::markAsUnread}},
    action_markAsImportant{new Kube::ControllerAction{this, &MailController::markAsImportant}},
    action_moveToTrash{new Kube::ControllerAction{this, &MailController::moveToTrash}},
    action_restoreFromTrash{new Kube::ControllerAction{this, &MailController::restoreFromTrash}},
    action_remove{new Kube::ControllerAction{this, &MailController::remove}},
    action_moveToFolder{new Kube::ControllerAction{this, &MailController::moveToFolder}}
{
    QObject::connect(this, &MailController::mailChanged, &MailController::updateActions);
    updateActions();
}

void MailController::updateActions()
{
    if (auto mail = getMail()) {
        action_moveToTrash->setEnabled(!mail->getTrash());
        action_restoreFromTrash->setEnabled(mail->getTrash());
    }
}

void MailController::markAsRead()
{
    auto mail = getMail();
    mail->setUnread(false);
    SinkLog() << "Mark as read " << mail->identifier();
    run(Store::modify(*mail));
}

void MailController::markAsUnread()
{
    auto mail = getMail();
    mail->setUnread(true);
    SinkLog() << "Mark as unread " << mail->identifier();
    run(Store::modify(*mail));
}

void MailController::markAsImportant()
{
    auto mail = getMail();
    mail->setImportant(true);
    SinkLog() << "Mark as important " << mail->identifier();
    run(Store::modify(*mail));
}

void MailController::moveToTrash()
{
    auto mail = getMail();
    mail->setTrash(true);
    SinkLog() << "Move to trash " << mail->identifier();
    run(Store::modify(*mail));
}

void MailController::restoreFromTrash()
{
    auto mail = getMail();
    mail->setTrash(false);
    SinkLog() << "Restore from trash " << mail->identifier();
    run(Store::modify(*mail));
}

void MailController::remove()
{
    auto mail = getMail();
    mail->setTrash(true);
    SinkLog() << "Remove " << mail->identifier();
    run(Store::remove(*mail));
}

void MailController::moveToFolder()
{
    auto mail = getMail();
    auto targetFolder = getTargetFolder();
    mail->setFolder(*targetFolder);
    SinkLog() << "Moving to folder " << mail->identifier() << targetFolder->identifier();
    run(Store::modify(*mail));
}

