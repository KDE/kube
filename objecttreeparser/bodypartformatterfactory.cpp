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

#include "bodypartformatterfactory.h"
#include "bodypartformatterfactory_p.h"
#include "messageviewer_debug.h"

#include "interfaces/bodypartformatter.h"
#include "pluginloader.h"
#include "urlhandlermanager.h"

// KDE

// Qt
#include <QString>
#include <QStringList>

#include <assert.h>

using namespace MessageViewer::BodyPartFormatterFactoryPrivate;
using namespace MessageViewer;

namespace
{

DEFINE_PLUGIN_LOADER(BodyPartFormatterPluginLoader,
                     Interface::BodyPartFormatterPlugin,
                     "create_bodypart_formatter_plugin",
                     "messageviewer/plugins/bodypartformatter/")

}

BodyPartFormatterFactory *BodyPartFormatterFactory::mSelf = 0;

const BodyPartFormatterFactory *BodyPartFormatterFactory::instance()
{
    if (!mSelf) {
        mSelf = new BodyPartFormatterFactory();
    }
    return mSelf;
}

BodyPartFormatterFactory::BodyPartFormatterFactory()
{
    mSelf = this;
}

BodyPartFormatterFactory::~BodyPartFormatterFactory()
{
    mSelf = 0;
}

static TypeRegistry *all = 0;

static void insertBodyPartFormatter(const char *type, const char *subtype,
                                    const Interface::BodyPartFormatter *formatter)
{
    if (!type || !*type || !subtype || !*subtype || !formatter || !all) {
        return;
    }

    TypeRegistry::iterator type_it = all->find(type);
    if (type_it == all->end()) {
        qCDebug(MESSAGEVIEWER_LOG) << "BodyPartFormatterFactory: instantiating new Subtype Registry for \""
                                   << type << "\"";
        type_it = all->insert(std::make_pair(type, SubtypeRegistry())).first;
        assert(type_it != all->end());
    }

    SubtypeRegistry &subtype_reg = type_it->second;
    SubtypeRegistry::iterator subtype_it = subtype_reg.find(subtype);
    if (subtype_it != subtype_reg.end()) {
        qCDebug(MESSAGEVIEWER_LOG) << "BodyPartFormatterFactory: overwriting previously registered formatter for \""
                                   << type << "/" << subtype << "\"";
        subtype_reg.erase(subtype_it); subtype_it = subtype_reg.end();
    }

    subtype_reg.insert(std::make_pair(subtype, formatter));
}

static void loadPlugins()
{
    const BodyPartFormatterPluginLoader *pl = BodyPartFormatterPluginLoader::instance();
    if (!pl) {
        qCWarning(MESSAGEVIEWER_LOG) << "BodyPartFormatterFactory: cannot instantiate plugin loader!";
        return;
    }
    const QStringList types = pl->types();
    qCDebug(MESSAGEVIEWER_LOG) << "BodyPartFormatterFactory: found" << types.size() << "plugins.";
    for (QStringList::const_iterator it = types.begin(); it != types.end(); ++it) {
        const Interface::BodyPartFormatterPlugin *plugin = pl->createForName(*it);
        if (!plugin) {
            qCWarning(MESSAGEVIEWER_LOG) << "BodyPartFormatterFactory: plugin" << *it << "is not valid!";
            continue;
        }
        const Interface::BodyPartFormatter *bfp;
        for (int i = 0; (bfp = plugin->bodyPartFormatter(i)); ++i) {
            const char *type = plugin->type(i);
            if (!type || !*type) {
                qCWarning(MESSAGEVIEWER_LOG) << "BodyPartFormatterFactory: plugin" << *it
                                             << "returned empty type specification for index"
                                             << i;
                break;
            }
            const char *subtype = plugin->subtype(i);
            if (!subtype || !*subtype) {
                qCWarning(MESSAGEVIEWER_LOG) << "BodyPartFormatterFactory: plugin" << *it
                                             << "returned empty subtype specification for index"
                                             << i;
                break;
            }
            insertBodyPartFormatter(type, subtype, bfp);
        }
        const Interface::BodyPartURLHandler *handler;
        for (int i = 0; (handler = plugin->urlHandler(i)); ++i) {
            URLHandlerManager::instance()->registerHandler(handler);
        }
    }
}

static void setup()
{
    if (!all) {
        all = new TypeRegistry();
        messageviewer_create_builtin_bodypart_formatters(all);
        loadPlugins();
    }
}

const Interface::BodyPartFormatter *BodyPartFormatterFactory::createFor(const char *type, const char *subtype) const
{
    if (!type || !*type) {
        type = "*";    //krazy:exclude=doublequote_chars
    }
    if (!subtype || !*subtype) {
        subtype = "*";    //krazy:exclude=doublequote_chars
    }

    setup();
    assert(all);

    if (all->empty()) {
        return 0;
    }

    TypeRegistry::const_iterator type_it = all->find(type);
    if (type_it == all->end()) {
        type_it = all->find("*");
    }
    if (type_it == all->end()) {
        return 0;
    }

    const SubtypeRegistry &subtype_reg = type_it->second;
    if (subtype_reg.empty()) {
        return 0;
    }

    SubtypeRegistry::const_iterator subtype_it = subtype_reg.find(subtype);
    if (subtype_it == subtype_reg.end()) {
        subtype_it = subtype_reg.find("*");
    }
    if (subtype_it == subtype_reg.end()) {
        return 0;
    }

    if (!(*subtype_it).second) {
        qCWarning(MESSAGEVIEWER_LOG) << "BodyPartFormatterFactory: a null bodypart formatter sneaked in for \""
                                     << type << "/" << subtype << "\"!";
    }

    return (*subtype_it).second;
}

const Interface::BodyPartFormatter *BodyPartFormatterFactory::createFor(const QString &type, const QString &subtype) const
{
    return createFor(type.toLatin1(), subtype.toLatin1());
}

const Interface::BodyPartFormatter *BodyPartFormatterFactory::createFor(const QByteArray &type, const QByteArray &subtype) const
{
    return createFor(type.constData(), subtype.constData());
}
