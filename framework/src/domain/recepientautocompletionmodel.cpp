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
#include "recepientautocompletionmodel.h"

#include <QStandardItemModel>
#include <QSettings>
#include <QStandardPaths>
#include <QSet>
#include <QDebug>
#include <QTimer>
#include <sink/store.h>
#include <sink/applicationdomaintype.h>

using namespace Sink::ApplicationDomain;

RecipientAutocompletionModel::RecipientAutocompletionModel(QObject *parent)
    : QSortFilterProxyModel(),
    mSourceModel(new QStandardItemModel),
    mTimer(new QTimer)
{
    setSourceModel(mSourceModel.data());
    setDynamicSortFilter(true);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    mTimer->setSingleShot(true);
    QObject::connect(mTimer.data(), &QTimer::timeout, this, &RecipientAutocompletionModel::save);

    load();
}

RecipientAutocompletionModel::~RecipientAutocompletionModel()
{
    save();
}

static QString getPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/kube/recepientautocompletion.ini";
}

void RecipientAutocompletionModel::save()
{
    QSet<QString> list;
    for (int row = 0; row < mSourceModel->rowCount(); row++) {
        list << mSourceModel->item(row)->data(Text).toString();
    }

    qWarning() << "Path " <<  getPath();
    QSettings settings(getPath(), QSettings::IniFormat);
    settings.setValue("list", QStringList{list.toList()});
}

void RecipientAutocompletionModel::load()
{
    qWarning() << "Path " <<  getPath();
    QSettings settings(getPath(), QSettings::IniFormat);
    auto list = settings.value("list").toStringList();
    auto add = [] (const QString &n) {
        auto item = new QStandardItem{n};
        item->setData(n, Text);
        return item;
    };
    for (const auto &entry : list) {
        mSourceModel->appendRow(add(entry));
    }
    Sink::Query query;
    query.request<Contact::Fn>();
    query.request<Contact::Emails>();
    Sink::Store::fetchAll<Contact>(query)
        .then([this] (const QList<Contact::Ptr> &list) {
            for (const auto &c : list) {
                for (const auto &email : c->getEmails()) {
                    addToModel(email.email, c->getFn());
                }
            }
        }).exec();
}

QHash< int, QByteArray > RecipientAutocompletionModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Text] = "text";
    roles[Color] = "color";
    return roles;
}


bool RecipientAutocompletionModel::addToModel(const QString &address, const QString &name)
{
    auto add = [] (const QString &n) {
        auto item = new QStandardItem{n};
        item->setData(n, Text);
        return item;
    };
    auto formattedName = [&] () {
        if (name.isEmpty()) {
            return QString(address);
        }
        return QString("%1 <%2>").arg(QString(name), QString(address));
    }();
    auto matches = mSourceModel->findItems(formattedName);
    if (matches.isEmpty()) {
        mSourceModel->appendRow(add(formattedName));
        return true;
    }
    return false;
}

void RecipientAutocompletionModel::addEntry(const QByteArray &address, const QByteArray &name)
{
    if (addToModel(address, name)) {
        mTimer->start(100);
    }
}

void RecipientAutocompletionModel::setFilter(const QString &filter)
{
    setFilterWildcard("*" + filter +"*");
}
