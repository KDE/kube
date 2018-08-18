/*
    bodypartformatterfactory.cpp

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2004 Marc Mutz <mutz@kde.org>,
                       Ingo Kloecker <kloecker@kde.org>

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    KMail is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "bodypartformatterbasefactory.h"
#include "bodypartformatterbasefactory_p.h"
#include "mimetreeparser_debug.h"

// Qt

#include <assert.h>

using namespace MimeTreeParser;

BodyPartFormatterBaseFactoryPrivate::BodyPartFormatterBaseFactoryPrivate(BodyPartFormatterBaseFactory *factory)
    : q(factory)
    , all(nullptr)
{
}

BodyPartFormatterBaseFactoryPrivate::~BodyPartFormatterBaseFactoryPrivate()
{
    if (all) {
        delete all;
        all = nullptr;
    }
}

void BodyPartFormatterBaseFactoryPrivate::setup()
{
    if (!all) {
        all = new TypeRegistry();
        messageviewer_create_builtin_bodypart_formatters();
    }
}

void BodyPartFormatterBaseFactoryPrivate::insert(const char *type, const char *subtype, const Interface::BodyPartFormatter *formatter)
{
    if (!type || !*type || !subtype || !*subtype || !formatter || !all) {
        return;
    }

    TypeRegistry::iterator type_it = all->find(type);
    if (type_it == all->end()) {
        type_it = all->insert(std::make_pair(type, SubtypeRegistry())).first;
        assert(type_it != all->end());
    }

    SubtypeRegistry &subtype_reg = type_it->second;

    subtype_reg.insert(std::make_pair(subtype, formatter));
}

BodyPartFormatterBaseFactory::BodyPartFormatterBaseFactory()
    : d(new BodyPartFormatterBaseFactoryPrivate(this))
{
}

BodyPartFormatterBaseFactory::~BodyPartFormatterBaseFactory()
{
    delete d;
}

void BodyPartFormatterBaseFactory::insert(const char *type, const char *subtype, const Interface::BodyPartFormatter *formatter)
{
    d->insert(type, subtype, formatter);
}

const SubtypeRegistry &BodyPartFormatterBaseFactory::subtypeRegistry(const char *type) const
{
    if (!type || !*type) {
        type = "*";    //krazy:exclude=doublequote_chars
    }

    d->setup();
    assert(d->all);

    static SubtypeRegistry emptyRegistry;
    if (d->all->empty()) {
        return emptyRegistry;
    }

    TypeRegistry::const_iterator type_it = d->all->find(type);
    if (type_it == d->all->end()) {
        type_it = d->all->find("*");
    }
    if (type_it == d->all->end()) {
        return emptyRegistry;
    }

    const SubtypeRegistry &subtype_reg = type_it->second;
    if (subtype_reg.empty()) {
        return emptyRegistry;
    }
    return subtype_reg;
}

SubtypeRegistry::const_iterator BodyPartFormatterBaseFactory::createForIterator(const char *type, const char *subtype) const
{
    if (!type || !*type) {
        type = "*";    //krazy:exclude=doublequote_chars
    }
    if (!subtype || !*subtype) {
        subtype = "*";    //krazy:exclude=doublequote_chars
    }

    d->setup();
    assert(d->all);

    if (d->all->empty()) {
        return SubtypeRegistry::const_iterator();
    }

    TypeRegistry::const_iterator type_it = d->all->find(type);
    if (type_it == d->all->end()) {
        type_it = d->all->find("*");
    }
    if (type_it == d->all->end()) {
        return SubtypeRegistry::const_iterator();
    }

    const SubtypeRegistry &subtype_reg = type_it->second;
    if (subtype_reg.empty()) {
        return SubtypeRegistry::const_iterator();
    }

    SubtypeRegistry::const_iterator subtype_it = subtype_reg.find(subtype);
    qCWarning(MIMETREEPARSER_LOG) << type << subtype << subtype_reg.size();
    if (subtype_it == subtype_reg.end()) {
        subtype_it = subtype_reg.find("*");
    }
    if (subtype_it == subtype_reg.end()) {
        return SubtypeRegistry::const_iterator();
    }

    if (!(*subtype_it).second) {
        qCWarning(MIMETREEPARSER_LOG) << "BodyPartFormatterBaseFactory: a null bodypart formatter sneaked in for \""
                                      << type << "/" << subtype << "\"!";
    }

    return subtype_it;
}

