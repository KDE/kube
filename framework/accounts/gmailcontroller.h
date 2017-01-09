/*
  Copyright (C) 2017 Michael Bohlender, <michael.bohlender@kdemail.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#pragma once

#include <QObject>
#include <QString>
#include <QByteArray>

#include <domain/controller.h>

class GmailController : public Kube::Controller
{
    Q_OBJECT

    //Interface properties
    KUBE_CONTROLLER_PROPERTY(QString, Name, name)

    KUBE_CONTROLLER_PROPERTY(QString, EmailAddress, emailAddress)
    KUBE_CONTROLLER_PROPERTY(QString, Password, password)
    KUBE_CONTROLLER_PROPERTY(QString, IdentityName, identityName)

    //Actions
    KUBE_CONTROLLER_ACTION(create)
    KUBE_CONTROLLER_ACTION(modify)
    KUBE_CONTROLLER_ACTION(remove)

public:
    explicit GmailController();

public slots:
    void load(const QByteArray &id);

private:
    QByteArray m_accountId;
    QByteArray m_smtpId;
    QByteArray m_imapId;
    QByteArray m_identityId;
};

