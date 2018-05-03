/*
    Copyright (c) 2016 Christian Mollekopf <mollekopf@kolabsys.com>

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
#include <QByteArray>
#include <QSettings>
#include <QSharedPointer>

namespace Kube {

class KUBE_EXPORT Settings : public QObject {
    Q_OBJECT
    Q_PROPERTY(QByteArray identifier READ identifier WRITE setIdentifier)
public:
    Settings(QObject *parent = 0);
    Settings(const QByteArray &id, QObject *parent = 0);
    virtual ~Settings();
    Settings(const Settings&);

    void setIdentifier(const QByteArray &id);
    QByteArray identifier() const;

    Q_INVOKABLE void save();
    Q_INVOKABLE void remove();
private:
    void load();
    QSharedPointer<QSettings> getSettings();
    QByteArray mIdentifier;
    bool mLoaded;
};

class Account;
class Identity;
class Transport;

class ApplicationContext : public Settings
{
    Q_OBJECT
public:
    ApplicationContext();
    Account currentAccount() const;

};

class Account : public Settings
{
    Q_OBJECT
public:
    Account(const QByteArray &identifier);
    Identity primaryIdentity() const;
    QByteArray type() const;
};

class Identity : public Settings
{
    Q_OBJECT
public:
    Identity(const QByteArray &identifier);
    Transport transport() const;
};

class Transport : public Settings
{
    Q_OBJECT
public:
    Transport(const QByteArray &identifier);
    QByteArray username() const;
    QByteArray password() const;
    QByteArray server() const;
};

}

Q_DECLARE_METATYPE(Kube::Settings*);

