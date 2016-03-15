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
#include "accountsmodel.h"

#include <settings/settings.h>

#include <QVariant>

AccountsModel::AccountsModel(QObject *parent) : QAbstractListModel()
{
    Kube::Settings settings("accounts");
    mAccounts = settings.property("accounts").toStringList();
}

AccountsModel::~AccountsModel()
{

}

QHash< int, QByteArray > AccountsModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[Name] = "name";
    roles[Icon] = "icon";
    roles[AccountId] = "accountId";

    return roles;
}

QVariant AccountsModel::data(const QModelIndex &idx, int role) const
{
    const auto identifier = mAccounts.at(idx.row());
    Kube::Account accountSettings(identifier.toLatin1());
    switch (role) {
        case Name:
            return accountSettings.property("accountName").toString();
        case Icon:
            return accountSettings.property("icon").toString();
        case AccountId:
            return identifier;
    }
    return QVariant();
}

int AccountsModel::rowCount(const QModelIndex &idx) const
{
    return mAccounts.size();
}
