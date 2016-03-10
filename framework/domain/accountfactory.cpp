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
#include "accountfactory.h"

#include <QQmlComponent>
#include <QQmlEngine>
#include <QDebug>

#include <KPackage/PackageLoader>

AccountFactory::AccountFactory(QObject *parent)
    : QObject(parent)
{

}

QString AccountFactory::name() const
{
    return "Maildir";
}

QString AccountFactory::icon() const
{
    return "icon";
}

QVariant AccountFactory::ui() const
{
    return createComponent(getAccountType());
}

QByteArray AccountFactory::getAccountType() const
{
    return "maildir";
}

QString AccountFactory::uiPath() const
{
    auto accountType = getAccountType();
    auto package = KPackage::PackageLoader::self()->loadPackage("KPackage/GenericQML", "org.kube.accounts." + accountType);
    Q_ASSERT(package.isValid());
    return package.filePath("mainscript");
}

QVariant AccountFactory::createComponent(const QByteArray &accountType) const const
{
    qWarning() << "Trying to load accounts package " << accountType << mAccountId;
    auto engine = qmlEngine(this);
    Q_ASSERT(engine);
    auto component = new QQmlComponent(engine, QUrl::fromLocalFile(uiPath()), QQmlComponent::PreferSynchronous);
    for (const auto &error : component->errors()) {
        qWarning() << error.toString();
    }
    Q_ASSERT(component->isReady());
    return QVariant::fromValue(component);
}
