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

#include <QDebug>

#include <KPackage/PackageLoader>
#include <KPackage/Package>
#include <KPluginMetaData>

#include "settings/settings.h"
#include <sink/store.h>

AccountFactory::AccountFactory(QObject *parent)
    : QObject(parent)
{
}

QString AccountFactory::name() const
{
    if (mName.isEmpty()) {
        return tr("Account");
    }
    return mName;
}

void AccountFactory::setAccountId(const QString &accountId)
{
    mAccountId = accountId;
    Sink::Store::fetchOne<Sink::ApplicationDomain::SinkAccount>(Sink::Query().filter(accountId.toUtf8()))
        .syncThen<void, Sink::ApplicationDomain::SinkAccount>([this](const Sink::ApplicationDomain::SinkAccount &account) {
            mAccountType = account.getProperty("type").toByteArray();
            loadPackage();
        }).exec();
}

void AccountFactory::loadPackage()
{
    auto package = KPackage::PackageLoader::self()->loadPackage("KPackage/GenericQML", "org.kube.accounts." + mAccountType);
    if (!package.isValid()) {
        qWarning() << "Failed to load account package: " << "org.kube.accounts." + mAccountType;
        return;
    }
    Q_ASSERT(package.isValid());
    mUiPath = package.filePath("mainscript");
    mName = package.metadata().name();
    mIcon = package.metadata().iconName();
    emit accountLoaded();
}
