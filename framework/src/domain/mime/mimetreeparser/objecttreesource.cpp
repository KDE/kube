/*
  Copyright (C) 2009 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Copyright (c) 2009 Andras Mantia <andras@kdab.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "objecttreesource.h"

#include <otp/attachmentstrategy.h>
#include <otp/bodypartformatterbasefactory.h>
#include <otp/messagepart.h>

class ObjectSourcePrivate
{
public:
    ObjectSourcePrivate()
        : mAllowDecryption(true)
        , mPreferredMode(MimeTreeParser::Util::Html)
    {

    }
    MimeTreeParser::BodyPartFormatterBaseFactory mBodyPartFormatterBaseFactory;
    bool mAllowDecryption;
    MimeTreeParser::Util::HtmlMode mPreferredMode;
};

ObjectTreeSource::ObjectTreeSource()
        : MimeTreeParser::Interface::ObjectTreeSource()
        , d(new ObjectSourcePrivate)
    {
    }

ObjectTreeSource::~ObjectTreeSource()
{
    delete d;
}

void ObjectTreeSource::setAllowDecryption(bool allowDecryption)
{
    d->mAllowDecryption = allowDecryption;
}


bool ObjectTreeSource::decryptMessage() const
{
    return d->mAllowDecryption;
}

const QTextCodec *ObjectTreeSource::overrideCodec()
{
    return Q_NULLPTR;
}

const MimeTreeParser::AttachmentStrategy *ObjectTreeSource::attachmentStrategy()
{
    return MimeTreeParser::AttachmentStrategy::smart();
}

QObject *ObjectTreeSource::sourceObject()
{
    return Q_NULLPTR;
}

void ObjectTreeSource::setHtmlMode(MimeTreeParser::Util::HtmlMode mode, const QList<MimeTreeParser::Util::HtmlMode> &availableModes)
{
      Q_UNUSED(mode);
      Q_UNUSED(availableModes);
}

MimeTreeParser::Util::HtmlMode ObjectTreeSource::preferredMode() const
{
    return d->mPreferredMode;
}

bool ObjectTreeSource::autoImportKeys() const
{
    return false;
}

const MimeTreeParser::BodyPartFormatterBaseFactory *ObjectTreeSource::bodyPartFormatterFactory()
{
    return &(d->mBodyPartFormatterBaseFactory);
}

