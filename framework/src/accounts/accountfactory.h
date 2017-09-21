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
 * A factory to instantiate account-plugins.
 */
class AccountFactory : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString accountId MEMBER mAccountId WRITE setAccountId);
    Q_PROPERTY(QString accountType MEMBER mAccountType WRITE setAccountType);
    Q_PROPERTY(QString name MEMBER mName READ name NOTIFY accountLoaded);
    Q_PROPERTY(QString icon MEMBER mIcon NOTIFY accountLoaded);
    Q_PROPERTY(QString uiPath MEMBER mUiPath NOTIFY accountLoaded);
    Q_PROPERTY(QString loginUi MEMBER mLoginUi NOTIFY accountLoaded);
public:
    explicit AccountFactory(QObject *parent = Q_NULLPTR);

    void setAccountId(const QString &);
    void setAccountType(const QString &);
    QString name() const;

signals:
    void accountLoaded();

private:
    void loadPackage();
    QString mAccountId;
    QString mName;
    QString mIcon;
    QString mUiPath;
    QString mLoginUi;
    QByteArray mAccountType;
};
