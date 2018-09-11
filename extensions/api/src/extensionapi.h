/*
    Copyright (c) 2018 Christian Mollekopf <mollekopf@kolabsys.com>

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

#include <QObject>

class ExtensionApi : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE void forwardMail(const QVariantMap &map);
    Q_INVOKABLE void storeSecret(const QByteArray &accountId, const QByteArray &keyLookupName, const QVariantMap &secret);
    Q_INVOKABLE void loadSecret(const QByteArray &accountId);

signals:
    void secretAvailable(const QByteArray &accountId,const QVariantMap &secret);
};
