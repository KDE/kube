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
#include "settings.h"

#include <QDebug>
#include <QStandardPaths>
#include <QMetaObject>
#include <QMetaProperty>

using namespace Kube;

Settings::Settings(QObject *parent)
    : QObject(parent)
{

}

Settings::Settings(const QByteArray &id, QObject *parent)
    : QObject(parent),
    mIdentifier(id)
{
    load();
}

Settings::Settings(const Settings &other)
    : QObject(other.parent()),
    mIdentifier(other.mIdentifier)
{
    load();
}

Settings::~Settings()
{
    // save();
}

QSharedPointer<QSettings> Settings::getSettings()
{
    return QSharedPointer<QSettings>::create(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QString("/kube/%1.ini").arg(QString::fromLatin1(mIdentifier)), QSettings::IniFormat);
}

void Settings::save()
{
    qWarning() << "Saving" << mIdentifier;
    auto settings = getSettings();
    for (int i = metaObject()->propertyOffset(); i < metaObject()->propertyCount(); i++) {
        const auto p = metaObject()->property(i).name();
        qWarning() << "setting " << p << property(p);
        settings->setValue(p, property(p));
    }
    settings->sync();
}

void Settings::load()
{
    qWarning() << "loading" << mIdentifier;
    for (int i = metaObject()->propertyOffset(); i < metaObject()->propertyCount(); i++) {
        auto p = metaObject()->property(i).name();
        setProperty(p, QVariant());
    }
    auto settings = getSettings();
    for (const auto &p : settings->allKeys()) {
        qWarning() << "loading " << p << settings->value(p);
        setProperty(p.toLatin1(), settings->value(p));
    }
}

void Settings::setIdentifier(const QByteArray &id)
{
    mIdentifier = id;
    load();
}

QByteArray Settings::identifier() const
{
    return mIdentifier;
}

ApplicationContext::ApplicationContext()
    : Settings("applicationcontext")
{

}

Account ApplicationContext::currentAccount() const
{
    return Account(property("currentAccountId").toByteArray());
}

Account::Account(const QByteArray &identifier)
    : Settings("account." + identifier)
{

}

Identity Account::primaryIdentity() const
{
    return Identity(property("primaryIdentityId").toByteArray());
}

Identity::Identity(const QByteArray &identifier)
    : Settings("identity." + identifier)
{

}

Transport Identity::transport() const
{
    return Transport(property("transportId").toByteArray());
}

Transport::Transport(const QByteArray &identifier)
    : Settings("transport." + identifier)
{

}

QByteArray Transport::username() const
{
    return property("username").toByteArray();
}

QByteArray Transport::password() const
{
    return property("password").toByteArray();
}

QByteArray Transport::server() const
{
    return property("server").toByteArray();
}
