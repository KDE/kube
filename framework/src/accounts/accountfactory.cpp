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
#include <QtQml>
#include <QFileInfo>
#include <QJsonDocument>
#include <QFile>

#include <sink/store.h>
#include <sink/log.h>

AccountFactory::AccountFactory(QObject *parent)
    : QObject(parent)
{
}

void AccountFactory::setAccountId(const QString &accountId)
{
    mAccountId = accountId;
    if (!accountId.isEmpty()) {
        Sink::Store::fetchOne<Sink::ApplicationDomain::SinkAccount>(Sink::Query().filter(accountId.toUtf8()))
            .then([this](const Sink::ApplicationDomain::SinkAccount &account) {
                mAccountType = account.getProperty("type").toByteArray();
                loadPackage();
            }).onError([=](const KAsync::Error &error) {
                SinkError() << "Failed to load the account: " << accountId << error;
            }).exec();
    }
}

void AccountFactory::setAccountType(const QString &type)
{
    mAccountType = type.toLatin1();
    loadPackage();
}

void AccountFactory::loadPackage()
{
    auto engine = QtQml::qmlEngine(this);
    Q_ASSERT(engine);
    const QString pluginPath = [&] {
        for (const auto &p : engine->importPathList()) {
            const auto path = p + QString::fromLatin1("/org/kube/accounts/") + mAccountType;
            if (QFileInfo::exists(path)) {
                return path;
            }
        }
        return QString{};
    }();
    mUiPath.clear();
    mLoginUi.clear();
    mAccountName.clear();
    mRequiresKeyring = false;
    if (pluginPath.isEmpty()) {
        SinkWarning() << "Failed to load account package: " << "org.kube.accounts." + mAccountType;
    } else {
        mUiPath = QUrl::fromLocalFile(pluginPath + "/AccountSettings.qml");
        mLoginUi = QUrl::fromLocalFile(pluginPath + "/Login.qml");
        mAccountName = mAccountType;
        if (QFileInfo::exists(pluginPath + "/metadata.json")) {
            QFile file{pluginPath + "/metadata.json"};
            file.open(QIODevice::ReadOnly);
            auto json = QJsonDocument::fromJson(file.readAll());
            mRequiresKeyring = json.object().value("RequiresKeyring").toBool(true);
            mAccountName = json.object().value("Name").toString();
        }
    }
    emit accountLoaded();
}
