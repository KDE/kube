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
#include <QJsonDocument>
#include <QJsonObject>

using namespace Kube;

ExtensionModel::ExtensionModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    sort(0, Qt::DescendingOrder);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    QTimer::singleShot(0, this, &ExtensionModel::load);
}

QHash<int, QByteArray> ExtensionModel::roleNames() const
{
    return {
        {Name, "name"},
        {Tooltip, "tooltip"},
        {Icon, "icon"}
    };
}

void ExtensionModel::load()
{
    auto model = new QStandardItemModel(this);

    auto engine = qmlEngine(this);
    Q_ASSERT(engine);
    for (const auto &path : engine->importPathList()) {
        QDir dir{path + "/org/kube/views"};
        for (const auto &pluginName : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            const auto pluginPath = dir.path() + "/" + pluginName;
            mPaths.insert(pluginName, pluginPath);
            auto item = new QStandardItem;
            item->setData(pluginName, Name);
            item->setData(pluginName, Tooltip);
            item->setData("kdocumentinfo-inverted", Icon);

            if (QFileInfo::exists(pluginPath + "/metadata.json")) {
                QFile file{pluginPath + "/metadata.json"};
                file.open(QIODevice::ReadOnly);
                auto json = QJsonDocument::fromJson(file.readAll());
                auto map = json.object().toVariantMap();
                item->setData(map.value("icon").toString(), Icon);
                item->setData(map.value("tooltip").toString(), Tooltip);
                if (map.value("hidden", false).toBool()) {
                    delete item;
                    continue;
                }
            }

            model->appendRow(item);
        }
    }
    setSourceModel(model);
}

QString ExtensionModel::findSource(const QString &extensionName, const QString &sourceName)
{
    if (mPaths.isEmpty()) {
        load();
    }
    return mPaths.value(extensionName) + "/" + sourceName;
}

void ExtensionModel::setSortOrder(const QVariantList &order)
{
    mSortOrder.clear();
    for (const auto &e : order) {
        mSortOrder << e.toString();
    }
}

QVariantList ExtensionModel::sortOrder() const
{
    return {};
}

QVariant ExtensionModel::data(const QModelIndex &idx, int role) const
{
    return QSortFilterProxyModel::data(idx, role);
}

bool ExtensionModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    auto leftIndex = mSortOrder.indexOf(left.data(Name).toString());
    auto rightIndex = mSortOrder.indexOf(right.data(Name).toString());
    if (leftIndex >= 0 && rightIndex >= 0) {
        //Higher index is less than
        return leftIndex > rightIndex;
    }
    if (leftIndex < 0 && rightIndex < 0) {
        return QSortFilterProxyModel::lessThan(left, right);
    }
    return leftIndex < rightIndex;
}
