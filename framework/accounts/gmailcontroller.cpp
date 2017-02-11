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

#include "gmailcontroller.h"

#include <sink/store.h>

using namespace Sink;
using namespace Sink::ApplicationDomain;

GmailController::GmailController() : Kube::Controller(),
action_create{new Kube::ControllerAction{this, &GmailController::create}},
action_modify{new Kube::ControllerAction{this, &GmailController::modify}},
action_remove{new Kube::ControllerAction{this, &GmailController::remove}}
{

}

void GmailController::create() {

    //account
    auto account = ApplicationDomainType::createEntity<SinkAccount>();
    account.setProperty("type", "imap");
    account.setProperty("name", getName());
    Store::create(account).exec().waitForFinished();

    //imap
    auto resource =  ApplicationDomainType::createEntity<SinkResource>();
    resource.setResourceType("sink.imap");
    resource.setAccount(account);
    resource.setProperty("server","imaps://imap.gmail.com:993");
    resource.setProperty("username", getEmailAddress());
    resource.setProperty("password", getPassword());
    Store::create(resource).exec().waitForFinished();

    //smtp
    resource = ApplicationDomainType::createEntity<SinkResource>();
    resource.setResourceType("sink.mailtransport");
    resource.setAccount(account);
    resource.setProperty("server", "smtps://smtp.gmail.com:465");
    resource.setProperty("username", getEmailAddress());
    resource.setProperty("password", getPassword());
    Store::create(resource).exec().waitForFinished();

    //identity
    auto identity = ApplicationDomainType::createEntity<Identity>();
    m_identityId = identity.identifier();
    identity.setAccount(account);
    identity.setName(getIdentityName());
    identity.setAddress(getEmailAddress());
    Store::create(identity).exec();
}

void GmailController::modify() {
    //TODO
}

void GmailController::remove() {
    SinkAccount account(m_accountId);
    Store::remove(account).exec();
}

void GmailController::load(const QByteArray &id) {

    m_accountId = id;

    Store::fetchOne<SinkAccount>(Query().filter(m_accountId))
    .then([this](const SinkAccount &account) {
        setName(account.getName());
    }).exec();

    //TODO
}

