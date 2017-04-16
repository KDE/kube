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
#pragma once

#include <QObject>
#include "controller.h"
#include "sink/applicationdomaintype.h"

class MailController : public Kube::Controller
{
    Q_OBJECT
    //Use this instead of mail property to get overall status of thread.
    KUBE_CONTROLLER_PROPERTY(bool, Unread, unread)
    KUBE_CONTROLLER_PROPERTY(bool, Important, important)
    KUBE_CONTROLLER_PROPERTY(bool, Trash, trash)
    KUBE_CONTROLLER_PROPERTY(bool, Draft, draft)

    KUBE_CONTROLLER_PROPERTY(Sink::ApplicationDomain::Mail::Ptr, Mail, mail)
    KUBE_CONTROLLER_PROPERTY(Sink::ApplicationDomain::Folder::Ptr, TargetFolder, targetFolder)
    KUBE_CONTROLLER_PROPERTY(bool, OperateOnThreads, operateOnThreads)
    KUBE_CONTROLLER_ACTION(markAsRead)
    KUBE_CONTROLLER_ACTION(markAsUnread)
    KUBE_CONTROLLER_ACTION(markAsImportant)
    KUBE_CONTROLLER_ACTION(toggleImportant)
    KUBE_CONTROLLER_ACTION(moveToTrash)
    KUBE_CONTROLLER_ACTION(restoreFromTrash)
    KUBE_CONTROLLER_ACTION(remove)
    KUBE_CONTROLLER_ACTION(moveToFolder)

public:
    explicit MailController();
private slots:
    void updateActions();
    void runModification(const std::function<void(Sink::ApplicationDomain::Mail &)> &f);
};
