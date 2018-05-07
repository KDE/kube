/*  -*- c++ -*-
    bodypartformatter.cpp

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2003 Marc Mutz <mutz@kde.org>

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

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

#include "mimetreeparser_debug.h"

#include "applicationpgpencrypted.h"
#include "applicationpkcs7mime.h"
#include "mailman.h"
#include "multipartalternative.h"
#include "multipartencrypted.h"
#include "multipartsigned.h"
#include "texthtml.h"
#include "textplain.h"

#include "bodypartformatter.h"
#include "bodypart.h"

#include "bodypartformatterbasefactory.h"
#include "bodypartformatterbasefactory_p.h"

#include "objecttreeparser.h"
#include "messagepart.h"

#include <KMime/Content>

using namespace MimeTreeParser;

namespace
{
class AnyTypeBodyPartFormatter
    : public MimeTreeParser::Interface::BodyPartFormatter
{
    static const AnyTypeBodyPartFormatter *self;
public:
    static const MimeTreeParser::Interface::BodyPartFormatter *create()
    {
        if (!self) {
            self = new AnyTypeBodyPartFormatter();
        }
        return self;
    }
};

const AnyTypeBodyPartFormatter *AnyTypeBodyPartFormatter::self = nullptr;

class MessageRfc822BodyPartFormatter
    : public MimeTreeParser::Interface::BodyPartFormatter
{
    static const MessageRfc822BodyPartFormatter *self;
public:
    MessagePart::Ptr process(Interface::BodyPart &part) const Q_DECL_OVERRIDE
    {
        return MessagePart::Ptr(new EncapsulatedRfc822MessagePart(part.objectTreeParser(), part.content(), part.content()->bodyAsMessage()));
    }

    static const MimeTreeParser::Interface::BodyPartFormatter *create()
    {
        if (!self) {
            self = new MessageRfc822BodyPartFormatter();
        }
        return self;
    }
};

const MessageRfc822BodyPartFormatter *MessageRfc822BodyPartFormatter::self;

class HeadersBodyPartFormatter
    : public MimeTreeParser::Interface::BodyPartFormatter
{
    static const HeadersBodyPartFormatter *self;
public:
    MessagePart::Ptr process(Interface::BodyPart &part) const Q_DECL_OVERRIDE
    {
        return MessagePart::Ptr(new HeadersPart(part.objectTreeParser(), part.content()));
    }

    static const MimeTreeParser::Interface::BodyPartFormatter *create() {
        if (!self) {
            self = new HeadersBodyPartFormatter();
        }
        return self;
    }
};

const HeadersBodyPartFormatter *HeadersBodyPartFormatter::self = nullptr;

class MultiPartRelatedBodyPartFormatter: public MimeTreeParser::Interface::BodyPartFormatter {
    static const MultiPartRelatedBodyPartFormatter *self;
public:

    QVector<MessagePart::Ptr> processList(Interface::BodyPart &part) const Q_DECL_OVERRIDE {
        if (part.content()->contents().isEmpty()) {
            return {};
        }
        //We rely on the order of the parts.
        //Theoretically there could also be a Start parameter which would break this..
        //https://tools.ietf.org/html/rfc2387#section-4

        //We want to display attachments even if displayed inline.
        QVector<MessagePart::Ptr> list;
        list.append(MimeMessagePart::Ptr(new MimeMessagePart(part.objectTreeParser(), part.content()->contents().at(0), true)));
        for (int i = 1; i < part.content()->contents().size(); i++) {
            auto p = part.content()->contents().at(i);
            if (KMime::isAttachment(p)) {
                list.append(MimeMessagePart::Ptr(new MimeMessagePart(part.objectTreeParser(), p, true)));
            }
        }
        return list;
    }

    static const MimeTreeParser::Interface::BodyPartFormatter *create() {
        if (!self) {
            self = new MultiPartRelatedBodyPartFormatter();
        }
        return self;
    }
};

const MultiPartRelatedBodyPartFormatter *MultiPartRelatedBodyPartFormatter::self = nullptr;

class MultiPartMixedBodyPartFormatter: public MimeTreeParser::Interface::BodyPartFormatter {
    static const MultiPartMixedBodyPartFormatter *self;
public:
    MessagePart::Ptr process(Interface::BodyPart &part) const Q_DECL_OVERRIDE {
        if (part.content()->contents().isEmpty()) {
            return {};
        }
        return MimeMessagePart::Ptr(new MimeMessagePart(part.objectTreeParser(), part.content()->contents().at(0), false));
    }

    static const MimeTreeParser::Interface::BodyPartFormatter *create() {
        if (!self) {
            self = new MultiPartMixedBodyPartFormatter();
        }
        return self;
    }
};

const MultiPartMixedBodyPartFormatter *MultiPartMixedBodyPartFormatter::self = nullptr;

} // anon namespace

void BodyPartFormatterBaseFactoryPrivate::messageviewer_create_builtin_bodypart_formatters()
{
    insert("application", "octet-stream", AnyTypeBodyPartFormatter::create());
    insert("application", "pgp", TextPlainBodyPartFormatter::create());
    insert("application", "pkcs7-mime", ApplicationPkcs7MimeBodyPartFormatter::create());
    insert("application", "x-pkcs7-mime", ApplicationPkcs7MimeBodyPartFormatter::create());
    insert("application", "pgp-encrypted", ApplicationPGPEncryptedBodyPartFormatter::create());
    insert("application", "*", AnyTypeBodyPartFormatter::create());

    insert("text", "html", TextHtmlBodyPartFormatter::create());
    insert("text", "rtf", AnyTypeBodyPartFormatter::create());
    insert("text", "plain", MailmanBodyPartFormatter::create());
    insert("text", "plain", TextPlainBodyPartFormatter::create());
    insert("text", "rfc822-headers", HeadersBodyPartFormatter::create());
    insert("text", "*", MailmanBodyPartFormatter::create());
    insert("text", "*", TextPlainBodyPartFormatter::create());

    insert("image", "*", AnyTypeBodyPartFormatter::create());

    insert("message", "rfc822", MessageRfc822BodyPartFormatter::create());
    insert("message", "*", AnyTypeBodyPartFormatter::create());

    insert("multipart", "alternative", MultiPartAlternativeBodyPartFormatter::create());
    insert("multipart", "encrypted", MultiPartEncryptedBodyPartFormatter::create());
    insert("multipart", "signed", MultiPartSignedBodyPartFormatter::create());
    insert("multipart", "related", MultiPartRelatedBodyPartFormatter::create());
    insert("multipart", "*", MultiPartMixedBodyPartFormatter::create());
    insert("*", "*", AnyTypeBodyPartFormatter::create());
}
