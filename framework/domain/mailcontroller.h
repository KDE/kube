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

#define KUBE_CONTROLLER_ACTION(NAME) \
    Q_PROPERTY (Kube::ControllerAction* NAME##Action READ NAME##Action CONSTANT) \
    private: QScopedPointer<Kube::ControllerAction> action_##NAME; \
    public: Kube::ControllerAction* NAME##Action() const { Q_ASSERT(action_##NAME); return action_##NAME.data(); } \
    private slots: void NAME(); \


class MailController : public Kube::Controller
{
    Q_OBJECT
    KUBE_CONTROLLER_PROPERTY(Sink::ApplicationDomain::Mail::Ptr, Mail, mail)
    KUBE_CONTROLLER_ACTION(markAsRead)
    KUBE_CONTROLLER_ACTION(moveToTrash)
    KUBE_CONTROLLER_ACTION(remove)

public:
    explicit MailController();
};
