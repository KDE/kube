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
#include "context.h"

#include <QDebug>
#include <QMetaProperty>

using namespace Kube;

Context::Context(QObject *parent)
    : QObject(parent)
{

}

Context::Context(const Context &other)
    : QObject()
{
    *this = other;
}

Context &Context::operator=(const Context &other)
{
    for (const auto &p : other.availableProperties()) {
        setProperty(p, other.property(p));
    }
    return *this;
}

void Context::clear()
{
    auto meta = metaObject();
    for (auto i = meta->propertyOffset(); i < meta->propertyCount(); i++) {
        auto property = meta->property(i);
        setProperty(property.name(), QVariant());
    }
    for (const auto &p : dynamicPropertyNames()) {
        setProperty(p, QVariant());
    }
}

QSet<QByteArray> Context::availableProperties() const
{
    QSet<QByteArray> names;
    auto meta = metaObject();
    for (auto i = meta->propertyOffset(); i < meta->propertyCount(); i++) {
        auto property = meta->property(i);
        names << property.name();
    }
    for (const auto &p : dynamicPropertyNames()) {
        names << p;
    }
    return names;
}

QDebug operator<<(QDebug dbg, const Kube::Context &context)
{
    dbg << "Kube::Context {\n";
    auto metaObject = context.metaObject();
    for (auto i = metaObject->propertyOffset(); i < metaObject->propertyCount(); i++) {
        auto property = metaObject->property(i);
        dbg << property.name() << context.property(property.name()) << "\n";
    }
    for (const auto &p : context.dynamicPropertyNames()) {
        dbg << p << context.property(p) << "\n";
    }
    dbg << "\n}";
    return dbg;
}

QDebug operator<<(QDebug dbg, const Kube::ContextWrapper &context)
{
    dbg << context.context;
    return dbg;
}
