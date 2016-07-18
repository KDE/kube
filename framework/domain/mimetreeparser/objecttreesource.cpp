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

#include <MimeTreeParser/AttachmentStrategy>
#include <MimeTreeParser/BodyPartFormatterBaseFactory>
#include <MimeTreeParser/MessagePart>
#include <MimeTreeParser/MessagePartRenderer>

class ObjectSourcePrivate
{
public:
    ObjectSourcePrivate()
        : mWriter(0)
        , mAllowDecryption(true)
        , mHtmlLoadExternal(true)
        , mHtmlMail(true)
    {

    }
    MimeTreeParser::HtmlWriter *mWriter;
    MimeTreeParser::BodyPartFormatterBaseFactory mBodyPartFormatterBaseFactory;
    bool mAllowDecryption;
    bool mHtmlLoadExternal;
    bool mHtmlMail;
};

ObjectTreeSource::ObjectTreeSource(MimeTreeParser::HtmlWriter *writer)
        : MimeTreeParser::Interface::ObjectTreeSource()
        , d(new ObjectSourcePrivate)
    {
        d->mWriter = writer;
    }

ObjectTreeSource::~ObjectTreeSource()
{
    delete d;
}

void ObjectTreeSource::setAllowDecryption(bool allowDecryption)
{
    d->mAllowDecryption = allowDecryption;
}

MimeTreeParser::HtmlWriter *ObjectTreeSource::htmlWriter()
{
    return d->mWriter;
}

bool ObjectTreeSource::htmlLoadExternal() const
{
    return d->mHtmlLoadExternal;
}

void ObjectTreeSource::setHtmlLoadExternal(bool loadExternal)
{
    d->mHtmlLoadExternal = loadExternal;
}

bool ObjectTreeSource::htmlMail() const
{
    return d->mHtmlMail;
}

void ObjectTreeSource::setHtmlMail(bool htmlMail)
{
    d->mHtmlMail = htmlMail;
}

bool ObjectTreeSource::decryptMessage() const
{
    return d->mAllowDecryption;
}

bool ObjectTreeSource::showSignatureDetails() const
{
    return true;
}

int ObjectTreeSource::levelQuote() const
{
    return 1;
}

const QTextCodec *ObjectTreeSource::overrideCodec()
{
    return Q_NULLPTR;
}

QString ObjectTreeSource::createMessageHeader(KMime::Message *message)
{
    return QString();
}

const MimeTreeParser::AttachmentStrategy *ObjectTreeSource::attachmentStrategy()
{
    return MimeTreeParser::AttachmentStrategy::smart();
}

QObject *ObjectTreeSource::sourceObject()
{
    return Q_NULLPTR;
}

void ObjectTreeSource::setHtmlMode(MimeTreeParser::Util::HtmlMode mode)
{
      Q_UNUSED(mode);
}

bool ObjectTreeSource::autoImportKeys() const
{
    return false;
}

bool ObjectTreeSource::showEmoticons() const
{
    return false;
}

bool ObjectTreeSource::showExpandQuotesMark() const
{
    return false;
}

const MimeTreeParser::BodyPartFormatterBaseFactory *ObjectTreeSource::bodyPartFormatterFactory()
{
    return &(d->mBodyPartFormatterBaseFactory);
}

MimeTreeParser::Interface::MessagePartRenderer::Ptr ObjectTreeSource::messagePartTheme(MimeTreeParser::Interface::MessagePart::Ptr msgPart)
{
    Q_UNUSED(msgPart);
    return MimeTreeParser::Interface::MessagePartRenderer::Ptr();
}
