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

#include <sink/applicationdomaintype.h>

ContactController::ContactController()
    : Kube::Controller(),
    action_save{new Kube::ControllerAction{this, &ContactController::save}}
{
    updateSaveAction();
}

void ContactController::save() {
    qWarning() << "Saving is not implemented";
}

void ContactController::updateSaveAction()
{
    saveAction()->setEnabled(!getName().isEmpty());
}

void ContactController::loadContact(const QVariant &contact)
{
    if (auto c = contact.value<Sink::ApplicationDomain::Contact::Ptr>()) {
        setName(c->getFn());
        QStringList emails;
        for (const auto &e : c->getEmails()) {
            emails << e;
        }
        setEmails(emails);
    }
}

QVariant ContactController::contact() const
{
    return QVariant{};
}

void ContactController::removeEmail(const QString &email)
{
    auto emails = getEmails();
    emails.removeAll(email);
    setEmails(emails);
}

void ContactController::addEmail(const QString &email)
{
    auto emails = getEmails();
    emails.append(email);
    setEmails(emails);
}
