/*
    partnodebodypart.cpp

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

#include "partnodebodypart.h"
#include "nodehelper.h"
#include "objecttreeparser.h"
#include "mimetreeparser_debug.h"
#include <KMime/Content>

#include <QTextCodec>
#include <QUrl>

using namespace MimeTreeParser;

static int serial = 0;

PartNodeBodyPart::PartNodeBodyPart(ObjectTreeParser *otp, KMime::Content *topLevelContent, KMime::Content *content,
                                   NodeHelper *nodeHelper)
    : Interface::BodyPart()
    , mTopLevelContent(topLevelContent)
    , mContent(content)
    , mNodeHelper(nodeHelper)
    , mObjectTreeParser(otp)
{}

QString PartNodeBodyPart::makeLink(const QString &path) const
{
    // FIXME: use a PRNG for the first arg, instead of a serial number
    return QStringLiteral("x-kmail:/bodypart/%1/%2/%3")
           .arg(serial++).arg(mContent->index().toString())
           .arg(QString::fromLatin1(QUrl::toPercentEncoding(path, "/")));
}

QString PartNodeBodyPart::asText() const
{
    if (!mContent->contentType()->isText()) {
        return QString();
    }
    return mContent->decodedText();
}

QByteArray PartNodeBodyPart::asBinary() const
{
    return mContent->decodedContent();
}

QString PartNodeBodyPart::contentTypeParameter(const char *param) const
{
    return mContent->contentType()->parameter(QString::fromLatin1(param));
}

QString PartNodeBodyPart::contentDescription() const
{
    return mContent->contentDescription()->asUnicodeString();
}

QString PartNodeBodyPart::contentDispositionParameter(const char *param) const
{
    return mContent->contentDisposition()->parameter(QString::fromLatin1(param));
}

