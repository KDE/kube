/*
 *  Copyright (C) 2017 Michael Bohlender, <michael.bohlender@kdemail.net>
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

#include "contactcontroller.h"

ContactController::ContactController()
    : Kube::Controller(),
    action_save{new Kube::ControllerAction{this, &ContactController::save}}
{
    loadContact("test");
    updateSaveAction();
}

void ContactController::save() {
    //TODO
}

void ContactController::updateSaveAction()
{
    saveAction()->setEnabled(!getName().isEmpty());
}

void ContactController::loadContact(const QVariant &contact)
{
    setName("Anita Rosenzweig");
    m_emails << "rosenzweig@kolabnow.com" << "wolfi@kolabnow.com";
}

void ContactController::removeEmail(const QString &email)
{
    m_emails.removeOne(email);
    emit emailsChanged();
}

void ContactController::addEmail(const QString &email)
{
    m_emails << email;
    emit emailsChanged();
}

QStringList ContactController::emails() const
{
    return m_emails;
}
