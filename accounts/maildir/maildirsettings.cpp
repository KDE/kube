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
#include "maildirsettings.h"

#include <sink/store.h>
#include <QDebug>

MaildirSettings::MaildirSettings(QObject *parent)
    : QObject(parent)
{
}

void MaildirSettings::setIdentifier(const QByteArray &id)
{
    mIdentifier = id;
    Sink::Store::fetchOne<Sink::ApplicationDomain::SinkResource>(Sink::Query::IdentityFilter(mIdentifier) + Sink::Query::RequestedProperties(QList<QByteArray>() << "path"))
        .then<void, Sink::ApplicationDomain::SinkResource>([this](const Sink::ApplicationDomain::SinkResource &resource) {
            setProperty("path", resource.getProperty("path"));
        }).exec();
}

QByteArray MaildirSettings::identifier() const
{
    return mIdentifier;
}

void MaildirSettings::save()
{
    if (!mIdentifier.isEmpty()) {
        Sink::ApplicationDomain::SinkResource resource(mIdentifier);
        resource.setProperty("path", property("path"));
        Sink::Store::modify(resource).exec();
    } else {
        Sink::ApplicationDomain::SinkResource resource;
        resource.setProperty("path", property("path"));
        Sink::Store::create(resource).exec();
    }
}

