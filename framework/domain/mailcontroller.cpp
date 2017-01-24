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
#include <sink/standardqueries.h>

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

void MailController::runModification(const std::function<void(ApplicationDomain::Mail &)> &f)
{
    if (auto mail = getMail()) {
        f(*mail);
        run(Store::modify(*mail));
    } else if (auto mail = getThreadLeader()) {
        f(*mail);
        run(Store::modify(Sink::StandardQueries::completeThread(*mail), *mail));
    }
}

void MailController::updateActions()
{
    auto mail = getMail();
    if (!mail) {
        mail= getThreadLeader();
    }
    if (mail) {
        action_moveToTrash->setEnabled(!mail->getTrash());
        action_restoreFromTrash->setEnabled(mail->getTrash());
    }
}

void MailController::markAsRead()
{
    runModification([] (ApplicationDomain::Mail &mail) {
        mail.setUnread(false);
        SinkLog() << "Mark as read " << mail.identifier();
    });
}

void MailController::markAsUnread()
{
    runModification([] (ApplicationDomain::Mail &mail) {
        mail.setUnread(true);
        SinkLog() << "Mark as unread " << mail.identifier();
    });
}

void MailController::markAsImportant()
{
    runModification([] (ApplicationDomain::Mail &mail) {
        mail.setImportant(true);
        SinkLog() << "Mark as important " << mail.identifier();
    });
}

void MailController::moveToTrash()
{
    runModification([] (ApplicationDomain::Mail &mail) {
        mail.setTrash(true);
        SinkLog() << "Move to trash " << mail.identifier();
    });
}

void MailController::restoreFromTrash()
{
    runModification([] (ApplicationDomain::Mail &mail) {
        mail.setTrash(false);
        SinkLog() << "Restore from trash " << mail.identifier();
    });
}

void MailController::remove()
{
    runModification([] (ApplicationDomain::Mail &mail) {
        mail.setTrash(true);
        SinkLog() << "Remove " << mail.identifier();
    });
}

void MailController::moveToFolder()
{
    runModification([&] (ApplicationDomain::Mail &mail) {
        auto targetFolder = getTargetFolder();
        mail.setFolder(*targetFolder);
        SinkLog() << "Moving to folder " << mail.identifier() << targetFolder->identifier();
    });
}

