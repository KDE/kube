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
#include <QSettings>

using namespace Kube;

Keyring::Keyring()
    : QObject()
{

}

bool Keyring::isUnlocked(const QByteArray &accountId)
{
    return false;
}

AccountKeyring::AccountKeyring(const QByteArray &accountId, QObject *parent)
    : QObject(parent),
    mAccountIdentifier(accountId)
{
}

void AccountKeyring::storePassword(const QByteArray &resourceId, const QString &password)
{
    QSettings settings{mAccountIdentifier + ".keyring", QSettings::IniFormat};
    settings.setValue(resourceId, password);
    Sink::SecretStore::instance().insert(resourceId, password);
}

void AccountKeyring::unlock()
{
    QSettings settings{mAccountIdentifier + ".keyring", QSettings::IniFormat};
    for (const auto &resourceId : settings.allKeys()) {
        Sink::SecretStore::instance().insert(resourceId.toLatin1(), settings.value(resourceId).toString());
    }
}

