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

#include <MessageViewer/AttachmentStrategy>

class ObjectSourcePrivate
{
public:
    ObjectSourcePrivate()
        : mWriter(0)
        , mCSSHelper(0)
        , mAllowDecryption(false)
        , mHtmlLoadExternal(false)
        , mHtmlMail(false)
    {

    }
    MessageViewer::HtmlWriter *mWriter;
    MessageViewer::CSSHelperBase *mCSSHelper;
    bool mAllowDecryption;
    bool mHtmlLoadExternal;
    bool mHtmlMail;
};

ObjectTreeSource::ObjectTreeSource(MessageViewer::HtmlWriter *writer,
                                   MessageViewer::CSSHelperBase *cssHelper)
        : MessageViewer::ObjectTreeSourceIf()
        , d(new ObjectSourcePrivate)
    {
        d->mWriter = writer;
        d->mCSSHelper = cssHelper;
    }

ObjectTreeSource::~ObjectTreeSource()
{
    delete d;
}

void ObjectTreeSource::setAllowDecryption(bool allowDecryption)
{
    d->mAllowDecryption = allowDecryption;
}

MessageViewer::HtmlWriter *ObjectTreeSource::htmlWriter()
{
    return d->mWriter;
}
MessageViewer::CSSHelperBase *ObjectTreeSource::cssHelper()
{
    return d->mCSSHelper;
}

bool ObjectTreeSource::htmlLoadExternal()
{
    return d->mHtmlLoadExternal;
}

void ObjectTreeSource::setHtmlLoadExternal(bool loadExternal)
{
    d->mHtmlLoadExternal = loadExternal;
}

bool ObjectTreeSource::htmlMail()
{
    return d->mHtmlMail;
}

void ObjectTreeSource::setHtmlMail(bool htmlMail)
{
    d->mHtmlMail = htmlMail;
}

bool ObjectTreeSource::decryptMessage()
{
    return d->mAllowDecryption;
}

bool ObjectTreeSource::showSignatureDetails() 
{
    return true;
}

int ObjectTreeSource::levelQuote() 
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

const MessageViewer::AttachmentStrategy *ObjectTreeSource::attachmentStrategy()
{
    return MessageViewer::AttachmentStrategy::smart();
}

QObject *ObjectTreeSource::sourceObject() 
{
    return Q_NULLPTR;
}

void ObjectTreeSource::setHtmlMode(MessageViewer::Util::HtmlMode mode)
{
      Q_UNUSED(mode);
}

bool ObjectTreeSource::autoImportKeys()
{
    return false;
}

bool ObjectTreeSource::showEmoticons()
{
    return false;
}

bool ObjectTreeSource::showExpandQuotesMark()
{
    return false;
}