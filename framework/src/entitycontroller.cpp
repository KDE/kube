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

#include "entitycontroller.h"

#include <sink/store.h>
#include <sink/log.h>

#include <QVariantMap>

using namespace Sink;
using namespace Sink::ApplicationDomain;

static QByteArrayList toByteArrayList(const QVariantList &list)
{
    QByteArrayList s;
    for (const auto &e : list) {
        s << e.toByteArray();
    }
    return s;
}

EntityController::EntityController(QObject *parent)
    : QObject(parent)
{

}

static KAsync::Job<void> createCalendar(const QByteArray &resourceId, const QVariantMap &object)
{
    using Sink::ApplicationDomain::Calendar;

    auto calendar = ApplicationDomainType::createEntity<Calendar>(resourceId);
    calendar.setName(object["name"].toString());
    if (object.contains("color")) {
        calendar.setColor(object["color"].toByteArray());
    }
    calendar.setEnabled(object["enabled"].toBool());
    if (object.contains("contentTypes")) {
        calendar.setContentTypes(toByteArrayList(object["contentTypes"].toList()));
    } else {
        calendar.setContentTypes({"event", "todo"});
    }
    return Sink::Store::create(calendar);
}

static KAsync::Job<QByteArray> findResource(const QVariantMap &map)
{
    if (map.contains("resource")) {
        return KAsync::value(map.value("resource").toByteArray());
    }

    Query query;
    //FIXME A resource that can store a type normally also provides it. Doesn't work for the mailtransportresource though.
    //Resource should probably just provide $type.store for all types.
    query.containsFilter<SinkResource::Capabilities>(map["type"].toByteArray());
    query.filter<SinkResource::Account>(map["account"].toByteArray());
    return Store::fetchOne<SinkResource>(query)
        .then([=](const SinkResource &resource) {
            return KAsync::value(resource.identifier());
        });
}

void EntityController::create(const QVariantMap &map)
{
    qDebug() << "Create entity " << map;
    auto job = findResource(map)
        .then([=](const QByteArray &resource) {
            if (map["type"].toString() == "calendar") {
                return createCalendar(resource, map["entity"].toMap());
            }
            return KAsync::error("Not handled");
        });
    job.exec();
}

void EntityController::remove(const QVariant &entity)
{
    //FIXME hardcoding calendar is not a great idea
    Sink::Store::remove<Sink::ApplicationDomain::Calendar>(*entity.value<Sink::ApplicationDomain::ApplicationDomainType::Ptr>()).exec();
}
