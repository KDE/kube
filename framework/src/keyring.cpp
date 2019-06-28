/*
    Copyright (c) 2017 Christian Mollekopf <mollekopf@kolabsys.com>

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
#include "keyring.h"

#include <sink/secretstore.h>
#include <QtGlobal>

using namespace Kube;

Q_GLOBAL_STATIC(Keyring, sKeyring);

Keyring::Keyring()
    : QObject()
{

}

Keyring *Keyring::instance()
{
    return sKeyring;
}

bool Keyring::isUnlocked(const QByteArray &accountId)
{
    return mUnlocked.contains(accountId);
}

void Keyring::unlock(const QByteArray &accountId)
{
    mUnlocked.insert(accountId);
}

AccountKeyring::AccountKeyring(const QByteArray &accountId, QObject *parent)
    : QObject(parent),
    mAccountIdentifier(accountId)
{
}

void AccountKeyring::storePassword(const QByteArray &resourceId, const QString &password)
{
    Sink::SecretStore::instance().insert(resourceId, password);
    Keyring::instance()->unlock(mAccountIdentifier);
}

void AccountKeyring::unlock()
{
    //TODO load passwords from an on disk keyring
}

