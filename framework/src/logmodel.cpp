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

#include "logmodel.h"

#include <QDebug>
#include <QDateTime>
#include <QStandardItem>

LogModel::LogModel(QObject *parent)
    : QStandardItemModel(parent)
{
    QByteArrayList roles{"type", "subtype", "timestamp", "message", "details", "entities", "resource"};

    int role = Qt::UserRole + 1;
    mRoles.insert("id", role);
    role++;
    for (const auto &r : roles) {
        mRoles.insert(r, role);
        role++;
    }

    QHash<int, QByteArray> roleNames;
    for (const auto r : mRoles.keys()) {
        roleNames.insert(mRoles[r], r);
    }
    setItemRoleNames(roleNames);
}

LogModel::~LogModel()
{

}

void LogModel::insert(const QVariantMap &message)
{

    if (rowCount() > 0) {
        auto i = item(0);
        const auto subtype = i->data(mRoles["subtype"]).toString();
        if (!subtype.isEmpty() && (subtype == message.value("subtype").toString())) {
            //TODO merge message into this entry
            return;
        }
    }

    auto item = new QStandardItem;
    auto addProperty = [&] (const QByteArray &key) {
        item->setData(message.value(key), mRoles[key]);
    };
    item->setData(QDateTime::currentDateTime(), mRoles["timestamp"]);
    addProperty("type");
    addProperty("subtype");
    addProperty("message");
    addProperty("details");
    addProperty("resource");
    addProperty("entities");
    insertRow(0, item);
}

