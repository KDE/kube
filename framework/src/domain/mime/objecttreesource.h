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

#ifndef MAILVIEWER_OBJECTTREEEMPTYSOURCE_H
#define MAILVIEWER_OBJECTTREEEMPTYSOURCE_H

#include <otp/objecttreesource.h>

class QString;

class ObjectSourcePrivate;
class ObjectTreeSource : public MimeTreeParser::Interface::ObjectTreeSource
{
public:
    ObjectTreeSource();
    virtual ~ObjectTreeSource();
    void setHtmlLoadExternal(bool loadExternal);
    bool decryptMessage() const Q_DECL_OVERRIDE;
    bool htmlLoadExternal() const Q_DECL_OVERRIDE;
    bool showSignatureDetails() const Q_DECL_OVERRIDE;
    void setHtmlMode(MimeTreeParser::Util::HtmlMode mode, const QList<MimeTreeParser::Util::HtmlMode> &availableModes) Q_DECL_OVERRIDE;
    MimeTreeParser::Util::HtmlMode preferredMode() const Q_DECL_OVERRIDE;
    void setAllowDecryption(bool allowDecryption);
    int levelQuote() const Q_DECL_OVERRIDE;
    const QTextCodec *overrideCodec() Q_DECL_OVERRIDE;
    QString createMessageHeader(KMime::Message *message) Q_DECL_OVERRIDE;
    const MimeTreeParser::AttachmentStrategy *attachmentStrategy() Q_DECL_OVERRIDE;
    /* MimeTreeParser::HtmlWriter *htmlWriter() Q_DECL_OVERRIDE; */
    QObject *sourceObject() Q_DECL_OVERRIDE;
    bool autoImportKeys() const Q_DECL_OVERRIDE;
    bool showEmoticons() const Q_DECL_OVERRIDE;
    bool showExpandQuotesMark() const Q_DECL_OVERRIDE;
    bool isPrinting() const Q_DECL_OVERRIDE;
    const MimeTreeParser::BodyPartFormatterBaseFactory *bodyPartFormatterFactory() Q_DECL_OVERRIDE;
private:
    ObjectSourcePrivate *const d;
};

#endif

