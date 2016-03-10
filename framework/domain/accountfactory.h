/*
    Copyright (c) 2016 Christian Mollekopf <mollekopf@kolabsystems.com>

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
#include <QVariant>

/**
 * A factory to instantiate accountp plugins.
 */
class AccountFactory : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString accountId MEMBER mAccountId);
    Q_PROPERTY(QString name READ name);
    Q_PROPERTY(QString icon READ icon);
    Q_PROPERTY(QVariant ui READ ui);
    Q_PROPERTY(QString uiPath READ uiPath);
public:
    explicit AccountFactory(QObject *parent = Q_NULLPTR);

    QString name() const;
    QString icon() const;
    QVariant ui() const;
    QString uiPath() const;

    Q_INVOKABLE QVariant createComponent(const QByteArray &accountType) const;
private:
    QByteArray getAccountType() const;
    QString mAccountId;
};
