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
#include <QStandardPaths>
#include <QDataStream>
#include <QSettings>
#include <QVariantMap>
#include <QVariant>
#include <QMap>
#include <QDebug>
#include "async.h"

using namespace Kube;

Q_GLOBAL_STATIC(Keyring, sKeyring);


static void storeSecret(const QByteArray &accountId, const std::vector<Crypto::Key> &keys, const QVariantMap &secret)
{
    QByteArray secretBA;
    QDataStream stream(&secretBA, QIODevice::WriteOnly);
    stream << secret;
    if (auto result = Crypto::signAndEncrypt(secretBA, keys, {})) {
        QSettings settings(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QString("/kube/secrets.ini"), QSettings::IniFormat);
        settings.setValue(accountId, result.value());
    } else {
        qWarning() << "Failed to encrypt account secret " << accountId;
    }
}

static QVariantMap loadSecret(const QByteArray &accountId)
{
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QString("/kube/secrets.ini"), QSettings::IniFormat);

    QByteArray secretBA;
    const auto encrypted = settings.value(accountId).value<QByteArray>();
    if (encrypted.isEmpty()) {
        return {};
    }
    Crypto::decryptAndVerify(Crypto::OpenPGP, encrypted, secretBA);

    QVariantMap map;
    QDataStream stream(&secretBA, QIODevice::ReadOnly);
    stream >> map;
    return map;
}


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

void Keyring::addPassword(const QByteArray &resourceId, const QString &password)
{
    Sink::SecretStore::instance().insert(resourceId, password);
}

void Keyring::tryUnlock(const QByteArray &accountId)
{
    if (isUnlocked(accountId)) {
        qInfo() << "Already unlocked" << accountId;
        return;
    }

    asyncRun<QVariantMap>(Keyring::instance(), [=] {
            return loadSecret(accountId);
        },
        [=](const QVariantMap &secrets) {
            for (const auto &resource : secrets.keys()) {
                //"accountSecret" is a magic value from the gpg extension for a key that is used for all resources of the same account.
                //We ignore it to avoid pretending the account is unlocked.
                if (resource == "accountSecret") {
                    continue;
                }
                auto secret = secrets.value(resource);
                if (secret.isValid()) {
                    Sink::SecretStore::instance().insert(resource.toLatin1(), secret.toString());
                    Keyring::instance()->unlock(accountId);
                } else {
                    qWarning() << "Found no stored secret for " << resource;
                }
            }
        });
}

AccountKeyring::AccountKeyring(const QByteArray &accountId, QObject *parent)
    : QObject(parent),
    mAccountIdentifier(accountId)
{
}

void AccountKeyring::addPassword(const QByteArray &resourceId, const QString &password)
{
    Keyring::instance()->addPassword(resourceId, password);
    Keyring::instance()->unlock(mAccountIdentifier);
    //FIXME: We keep track of secrets stored via this account for when we persist it,
    //because we have no other good means to do so.
    mAccountResources << resourceId;
}

void AccountKeyring::save(const std::vector<Crypto::Key> &keys)
{
    QVariantMap secrets;
    for (const auto &resourceId : mAccountResources) {
        secrets.insert(resourceId, Sink::SecretStore::instance().resourceSecret(resourceId));
    }
    storeSecret(mAccountIdentifier, keys, secrets);
}

void AccountKeyring::load()
{
    asyncRun<QVariantMap>(this, [=] {
            const auto secrets = loadSecret(mAccountIdentifier);
            return secrets;
        },
        [this](const QVariantMap &secrets) {
            for (const auto &resource : secrets.keys()) {
                //"accountSecret" is a magic value from the gpg extension for a key that is used for all resources of the same account.
                //We ignore it to avoid pretending the account is unlocked.
                if (resource == "accountSecret") {
                    continue;
                }
                auto secret = secrets.value(resource);
                if (secret.isValid()) {
                    qWarning() << "Found stored secret for " << resource;
                    addPassword(resource.toLatin1(), secret.toString());
                } else {
                    qWarning() << "Found no stored secret for " << resource;
                }
            }
            emit loaded();
        });
}
