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

#include "objecttreeemptysource.h"
#include "viewer/viewer_p.h"

#include "attachmentstrategy.h"

namespace MessageViewer
{

class EmptySourcePrivate
{
public:
    EmptySourcePrivate()
        : mAllowDecryption(false)
    {

    }
    bool mAllowDecryption;
};

EmptySource::EmptySource()
    : ObjectTreeSourceIf(),
      d(new MessageViewer::EmptySourcePrivate)
{
}

EmptySource::~EmptySource()
{
    delete d;
}

bool EmptySource::htmlMail()
{
    return true;
}

bool EmptySource::decryptMessage()
{
    return d->mAllowDecryption;
}

bool EmptySource::htmlLoadExternal()
{
    return false;
}

bool EmptySource::showSignatureDetails()
{
    return false;
}

void EmptySource::setHtmlMode(Util::HtmlMode mode)
{
    Q_UNUSED(mode);
}

void EmptySource::setAllowDecryption(bool allowDecryption)
{
    d->mAllowDecryption = allowDecryption;
}

int EmptySource::levelQuote()
{
    return 1;
}

const QTextCodec *EmptySource::overrideCodec()
{
    return 0;
}

QString EmptySource::createMessageHeader(KMime::Message *message)
{
    Q_UNUSED(message);
    return QString(); //do nothing
}

QObject *EmptySource::sourceObject()
{
    return 0;
}

const AttachmentStrategy *EmptySource::attachmentStrategy()
{
    return AttachmentStrategy::smart();
}

HtmlWriter *EmptySource::htmlWriter()
{
    return 0;
}

CSSHelper *EmptySource::cssHelper()
{
    return 0;
}

}

