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
#pragma once

#include "kube_export.h"
#include <QObject>
#include <QSet>

namespace Kube {

class KUBE_EXPORT Keyring : public QObject {
    Q_OBJECT
public:
    Keyring();
    static Keyring *instance();
    Q_INVOKABLE bool isUnlocked(const QByteArray &accountId);
    Q_INVOKABLE void unlock(const QByteArray &accountId);

private:
    Q_DISABLE_COPY(Keyring);
    QSet<QByteArray> mUnlocked;
};

class AccountKeyring : public QObject {
    Q_OBJECT
public:
    AccountKeyring(const QByteArray &accountId, QObject *parent = nullptr);
    void storePassword(const QByteArray &resourceId, const QString &password);
    void unlock();

private:
    Q_DISABLE_COPY(AccountKeyring);

    QByteArray mAccountIdentifier;
};

}
