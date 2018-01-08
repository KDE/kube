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
#include "extensionmodel.h"
#include <QStandardItemModel>
#include <QQmlEngine>
#include <QDir>
#include <QDebug>
#include <QTimer>

using namespace Kube;

ExtensionModel::ExtensionModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    QTimer::singleShot(0, this, &ExtensionModel::load);
}

QHash<int, QByteArray> ExtensionModel::roleNames() const
{
    return {
        {Name, "name"},
        {Tooltip, "tooltip"},
        {Icon, "icon"},
        {Source, "source"}
    };
}

void ExtensionModel::load()
{
    auto model = new QStandardItemModel(this);

    auto engine = qmlEngine(this);
    Q_ASSERT(engine);
    for (const auto &path : engine->importPathList()) {
        QDir dir{path + "/org/kube/viewextensions"};
        for (const auto &pluginName : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            auto viewPath = dir.path() + pluginName + "/View.qml";
            qWarning() << "Plugin path: " << dir.path() + pluginName + "/View.qml";
            auto item = new QStandardItem;
            item->setData(viewPath, Source);
            item->setData(pluginName, Name);
            item->setData(pluginName, Tooltip);
            item->setData("document-decrypt", Icon);
            model->appendRow(item);
        }
    }
    setSourceModel(model);
}

QVariant ExtensionModel::data(const QModelIndex &idx, int role) const
{
    return QSortFilterProxyModel::data(idx, role);
}

bool ExtensionModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    return QSortFilterProxyModel::lessThan(left, right);
}
