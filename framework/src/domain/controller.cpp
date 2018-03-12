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
#include "controller.h"

#include <QQmlEngine>
#include <QMetaProperty>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QUuid>
#include <sink/log.h>

using namespace Kube;

ControllerState::ControllerState()
    : QObject()
{
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
}

ControllerAction::ControllerAction()
    : ControllerState()
{
}

void ControllerAction::execute()
{
    emit triggered();
}

void Controller::clear()
{
    auto meta = metaObject();
    //We want to get the offset for this class, but clear the properties of all subclasses (thus staticMetaObject for the offset)
    for (auto i = staticMetaObject.propertyOffset(); i < meta->propertyCount(); i++) {
        auto property = meta->property(i);
        setProperty(property.name(), QVariant());
    }
    for (const auto &p : dynamicPropertyNames()) {
        setProperty(p, QVariant());
    }
}

void Controller::run(const KAsync::Job<void> &job)
{
    auto jobToExec = job;
    jobToExec.onError([] (const KAsync::Error &error) {
        SinkWarningCtx(Sink::Log::Context{"controller"}) << "Error while executing job: " << error.errorMessage;
    });
    //TODO handle error
    //TODO attach a log context to the execution that we can gather from the job?
    jobToExec.exec();
}


static void traverse(const QStandardItemModel *model, const std::function<bool(QStandardItem *item)> &f)
{
    auto root = model->invisibleRootItem();
    for (int row = 0; row < root->rowCount(); row++) {
        if (!f(root->child(row, 0))) {
            return;
        }
    }
}

ListPropertyController::ListPropertyController(const QStringList &roles)
    : QObject(),
    mModel(new QStandardItemModel)
{
    //Generate a set of roles for the names. We're not using any enum, so the actual role value doesn't matter.
    int role = Qt::UserRole + 1;
    mRoles.insert("id", role);
    role++;
    for (const auto &r : roles) {
        mRoles.insert(r, role);
        role++;
    }

    QHash<int, QByteArray> roleNames;
    for (const auto r : mRoles.keys()) {
        roleNames.insert(mRoles[r], r.toLatin1());
    }
    mModel->setItemRoleNames(roleNames);
}

void ListPropertyController::add(const QVariantMap &value)
{
    auto item = new QStandardItem;
    auto id = QUuid::createUuid().toByteArray();
    item->setData(id, mRoles["id"]);
    for (const auto &k : value.keys()) {
        item->setData(value.value(k), mRoles[k]);
    }
    mModel->appendRow(QList<QStandardItem*>() << item);
    if (mModel->rowCount() <= 1) {
        emit emptyChanged();
    }
    emit added(id, value);
}

void ListPropertyController::remove(const QByteArray &id)
{
    auto root = mModel->invisibleRootItem();
    const auto idRole = mRoles["id"];
    for (int row = 0; row < root->rowCount(); row++) {
        if (root->child(row, 0)->data(idRole).toByteArray() == id) {
            root->removeRow(row);
            break;
        }
    }
    emit removed(id);
    if (mModel->rowCount() <= 0) {
        emit emptyChanged();
    }
}

bool ListPropertyController::empty() const
{
    return mModel->rowCount() == 0;
}

void ListPropertyController::clear()
{
    mModel->clear();
}

QAbstractItemModel *ListPropertyController::model()
{
    QQmlEngine::setObjectOwnership(mModel.data(), QQmlEngine::CppOwnership);
    return mModel.data();
}

void ListPropertyController::setValue(const QByteArray &id, const QString &key, const QVariant &value)
{
    setValues(id, {{key, value}});
}

void ListPropertyController::setValues(const QByteArray &id, const QVariantMap &values)
{
    const auto idRole = mRoles["id"];
    ::traverse(mModel.data(), [&] (QStandardItem *item) {
        if (item->data(idRole).toByteArray() == id) {
            for (const auto &key : values.keys()) {
                item->setData(values.value(key), mRoles[key]);
            }
            return false;
        }
        return true;
    });
}

void ListPropertyController::traverse(const std::function<void(const QVariantMap &)> &f)
{
    ::traverse(mModel.data(), [&] (QStandardItem *item) {
        QVariantMap map;
        for (const auto &key : mRoles.keys()) {
            map.insert(key, item->data(mRoles[key]));
        }
        f(map);
        return true;
    });
}

