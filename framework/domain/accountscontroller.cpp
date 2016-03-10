/*
    Copyright (c) 2016 Michael Bohlender <michael.bohlender@kdemail.net>

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


#include "accountscontroller.h"

#include <settings/settings.h>

#include <QVariant>
#include <QUuid>
#include <QDebug>

AccountsController::AccountsController(QObject *parent) : QObject(parent)
{
    Kube::Settings settings("accounts");
    mAccounts = settings.property("accounts").toStringList();
    qWarning() << "Loaded accounts" << mAccounts;
}

void AccountsController::createAccount(const QString &accountType)
{
    auto identifier = QUuid::createUuid().toByteArray();
    Kube::Account accountSettings(identifier);
    accountSettings.setProperty("type", accountType);
    accountSettings.save();

    Kube::Settings settings("accounts");
    auto accounts = settings.property("accounts").toStringList();
    accounts.append(identifier);
    settings.setProperty("accounts", accounts);
    settings.save();

    //TODO setup sink resources etc via plugin

    qWarning() << "Created account " << identifier;
    mAccounts.append(identifier);
    emit accountsChanged();
}
