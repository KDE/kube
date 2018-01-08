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
#include "teststore.h"

#include <sink/store.h>

#include <QDebug>
#include <QVariant>

using namespace Kube;

static void iterateOverObjects(const QVariantList &list, std::function<void(const QVariantMap &)> callback)
{
    for (const auto &entry : list) {
        auto object = entry.toMap();
        callback(object);
    }
}

void TestStore::setup(const QVariantMap &map)
{
    iterateOverObjects(map.value("resources").toList(), [] (const QVariantMap &object) {
        auto resource = [&] {
            if (object["type"] == "dummy") {
                return Sink::ApplicationDomain::DummyResource::create(object["account"].toByteArray());
            }
            if (object["type"] == "mailtransport") {
                return Sink::ApplicationDomain::MailtransportResource::create(object["account"].toByteArray());
            }
            Q_ASSERT(false);
            return Sink::ApplicationDomain::SinkResource{};
        }();
        Sink::Store::create(resource).exec().waitForFinished();
    });

    iterateOverObjects(map.value("identities").toList(), [] (const QVariantMap &object) {
        auto identity = Sink::ApplicationDomain::Identity{};
        identity.setAccount(object["account"].toByteArray());
        identity.setAddress(object["address"].toString());
        identity.setName(object["name"].toString());
        Sink::Store::create(identity).exec().waitForFinished();
    });
}
