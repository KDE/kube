/*
    bodypartformatterfactory.h

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

#ifndef __MIMETREEPARSER_BODYPARTFORMATTERBASEFACTORY_H__
#define __MIMETREEPARSER_BODYPARTFORMATTERBASEFACTORY_H__

#include <map>
#include <QByteArray>

namespace MimeTreeParser
{

namespace Interface
{
class BodyPartFormatter;
}

struct ltstr {
    bool operator()(const char *s1, const char *s2) const
    {
        return qstricmp(s1, s2) < 0;
    }
};

typedef std::multimap<const char *, const Interface::BodyPartFormatter *, ltstr> SubtypeRegistry;
typedef std::map<const char *, MimeTreeParser::SubtypeRegistry, MimeTreeParser::ltstr> TypeRegistry;

class BodyPartFormatterBaseFactoryPrivate;

class BodyPartFormatterBaseFactory
{
public:
    BodyPartFormatterBaseFactory();
    virtual ~BodyPartFormatterBaseFactory();

    SubtypeRegistry::const_iterator createForIterator(const char *type, const char *subtype) const;
    const SubtypeRegistry &subtypeRegistry(const char *type) const;

protected:
    void insert(const char *type, const char *subtype, const Interface::BodyPartFormatter *formatter);
private:
    static BodyPartFormatterBaseFactory *mSelf;

    BodyPartFormatterBaseFactoryPrivate *d;
    friend class BodyPartFormatterBaseFactoryPrivate;
private:
    // disabled
    const BodyPartFormatterBaseFactory &operator=(const BodyPartFormatterBaseFactory &);
    BodyPartFormatterBaseFactory(const BodyPartFormatterBaseFactory &);
};

}

#endif // __MIMETREEPARSER_BODYPARTFORMATTERFACTORY_H__
