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

#include "objecttreeviewersource.h"
#include "viewer/viewer_p.h"
#include "widgets/htmlstatusbar.h"

namespace MessageViewer
{
MailViewerSource::MailViewerSource(ViewerPrivate *viewer) :
    ObjectTreeSourceIf(), mViewer(viewer)
{
}

MailViewerSource::~MailViewerSource()
{
}

bool MailViewerSource::htmlMail()
{
    return mViewer->htmlMail();
}

bool MailViewerSource::decryptMessage()
{
    return mViewer->decryptMessage();
}

bool MailViewerSource::htmlLoadExternal()
{
    return mViewer->htmlLoadExternal();
}

bool MailViewerSource::showSignatureDetails()
{
    return mViewer->mShowSignatureDetails;
}

void MailViewerSource::setHtmlMode(Util::HtmlMode mode)
{
    mViewer->mColorBar->setMode(mode);
}

int MailViewerSource::levelQuote()
{
    return mViewer->mLevelQuote;
}

const QTextCodec *MailViewerSource::overrideCodec()
{
    return mViewer->overrideCodec();
}

QString MailViewerSource::createMessageHeader(KMime::Message *message)
{
    return mViewer->writeMsgHeader(message);
}

QObject *MailViewerSource::sourceObject()
{
    return mViewer;
}

const AttachmentStrategy *MailViewerSource::attachmentStrategy()
{
    return mViewer->attachmentStrategy();
}

HtmlWriter *MailViewerSource::htmlWriter()
{
    return mViewer->htmlWriter();
}

CSSHelper *MailViewerSource::cssHelper()
{
    return mViewer->cssHelper();
}

}
