/*
 *  Copyright (C) 2017 Michael Bohlender, <michael.bohlender@kdemail.net>
 *  Copyright (C) 2018 Christian Mollekopf, <mollekopf@kolabsys.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#pragma once
#include "kube_export.h"

#include <QString>
#include <QDateTime>

#include <sink/applicationdomaintype.h>

#include "eventcontroller.h"

class KUBE_EXPORT InvitationController : public EventController
{
    Q_OBJECT

public:
    enum InvitationState {
        Unknown,
        Accepted,
        Declined,
    };
    Q_ENUM(InvitationState);

    KUBE_CONTROLLER_PROPERTY(QByteArray, Uid, uid)
    KUBE_CONTROLLER_PROPERTY(InvitationState, State, state)

    KUBE_CONTROLLER_ACTION(accept)
    KUBE_CONTROLLER_ACTION(decline)

public:
    explicit InvitationController();

    Q_INVOKABLE void loadICal(const QString &message);
};
