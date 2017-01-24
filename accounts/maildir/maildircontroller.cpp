/*
 *    Copyright (c) 2016 Michael Bohlender <michael.bohlender@kdemail.net>
 *    Copyright (c) 2016 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 *    This library is free software; you can redistribute it and/or modify it
 *    under the terms of the GNU Library General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or (at your
 *    option) any later version.
 *
 *    This library is distributed in the hope that it will be useful, but WITHOUT
 *    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 *    License for more details.
 *
 *    You should have received a copy of the GNU Library General Public License
 *    along with this library; see the file COPYING.LIB.  If not, write to the
 *    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *    02110-1301, USA.
 */

#include "maildircontroller.h"

#include <sink/store.h>

using namespace Sink;
using namespace Sink::ApplicationDomain;

MaildirController::MaildirController(QObject *parent)
: QObject(parent)
{
}

QByteArray MaildirController::accountIdentifier() const
{
    return mAccountIdentifier;
}

void MaildirController::setAccountIdentifier(const QByteArray &id)
{
    if (id.isEmpty()) {
        return;
    }
    mAccountIdentifier = id;

    //clear
    mIcon = QString();
    mName = QString();
    mPath = QUrl();

    //load
}

QUrl MaildirController::path() const
{
    return QUrl(mPath);
}

void MaildirController::setPath(const QUrl &path)
{
    auto normalizedPath = path.path();
    if (mPath != normalizedPath) {
        mPath = normalizedPath;
        emit pathChanged();
    }
}

void MaildirController::createAccount()
{
    auto account = ApplicationDomainType::createEntity<SinkAccount>();
    account.setProperty("type", "maildir");
    account.setProperty("name", mName);
    account.setProperty("icon", mIcon);
    Store::create(account).exec().waitForFinished();

    auto resource =  ApplicationDomainType::createEntity<SinkResource>();
    resource.setResourceType("sink.maildir");
    resource.setAccount(account);
    resource.setProperty("path", mPath);
    //TODO not yet implemented in resource? // resource.setProperty("readonly", readonly);
    Store::create(resource).exec().waitForFinished();
}

//TODO
void MaildirController::loadAccount(const QByteArray &id)
{
    Q_ASSERT(!mAccountIdentifier.isEmpty());
    Store::fetchOne<SinkAccount>(Query().filter(mAccountIdentifier))
    .then([this](const SinkAccount &account) {
        mIcon = account.getIcon();
        mName = account.getName();
        emit nameChanged();
        emit iconChanged();
    }).exec();
}

//TODO
void MaildirController::modifyAccount()
{
}

void MaildirController::deleteAccount()
{
    if (!mAccountIdentifier.isEmpty()) {
        SinkAccount account(mAccountIdentifier);
        Store::remove(account).exec();
    }
}
