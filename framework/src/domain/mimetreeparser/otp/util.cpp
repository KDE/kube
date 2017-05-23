/*
  Copyright (c) 2016 Sandro Knauß <sknauss@kde.org>

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

#include "util.h"

#include "mimetreeparser_debug.h"

#include "nodehelper.h"

#include <KMime/Content>

#include <QMimeDatabase>
#include <QString>

using namespace MimeTreeParser::Util;

bool MimeTreeParser::Util::isTypeBlacklisted(KMime::Content *node)
{
    const QByteArray mediaTypeLower = node->contentType()->mediaType().toLower();
    bool typeBlacklisted = mediaTypeLower == "multipart";
    if (!typeBlacklisted) {
        typeBlacklisted = KMime::isCryptoPart(node);
    }
    typeBlacklisted = typeBlacklisted || node == node->topLevel();
    const bool firstTextChildOfEncapsulatedMsg =
        mediaTypeLower == "text" &&
        node->contentType()->subType().toLower() == "plain" &&
        node->parent() && node->parent()->contentType()->mediaType().toLower() == "message";
    return  typeBlacklisted || firstTextChildOfEncapsulatedMsg;
}

QString MimeTreeParser::Util::labelForContent(KMime::Content *node)
{
    const QString name = node->contentType()->name();
    QString label = name.isEmpty() ? NodeHelper::fileName(node) : name;
    if (label.isEmpty()) {
        label = node->contentDescription()->asUnicodeString();
    }
    return label;
}

QMimeType MimeTreeParser::Util::mimetype(const QString &name)
{
    QMimeDatabase db;
    // consider the filename if mimetype cannot be found by content-type
    const auto mimeTypes = db.mimeTypesForFileName(name);
    for (const auto &mt : mimeTypes) {
        if (mt.name() != QLatin1String("application/octet-stream")) {
            return mt;
        }
    }

    // consider the attachment's contents if neither the Content-Type header
    // nor the filename give us a clue
    return db.mimeTypeForFile(name);
}

QString MimeTreeParser::Util::iconNameForMimetype(const QString &mimeType,
        const QString &fallbackFileName1,
        const QString &fallbackFileName2)
{
    QString fileName;
    QString tMimeType = mimeType;

    // convert non-registered types to registered types
    if (mimeType == QLatin1String("application/x-vnd.kolab.contact")) {
        tMimeType = QStringLiteral("text/x-vcard");
    } else if (mimeType == QLatin1String("application/x-vnd.kolab.event")) {
        tMimeType = QStringLiteral("application/x-vnd.akonadi.calendar.event");
    } else if (mimeType == QLatin1String("application/x-vnd.kolab.task")) {
        tMimeType = QStringLiteral("application/x-vnd.akonadi.calendar.todo");
    } else if (mimeType == QLatin1String("application/x-vnd.kolab.journal")) {
        tMimeType = QStringLiteral("application/x-vnd.akonadi.calendar.journal");
    } else if (mimeType == QLatin1String("application/x-vnd.kolab.note")) {
        tMimeType = QStringLiteral("application/x-vnd.akonadi.note");
    } else if (mimeType == QLatin1String("image/jpg")) {
        tMimeType = QStringLiteral("image/jpeg");
    }
    QMimeDatabase mimeDb;
    auto mime = mimeDb.mimeTypeForName(tMimeType);
    if (mime.isValid()) {
        fileName = mime.iconName();
    } else {
        fileName = QStringLiteral("unknown");
        if (!tMimeType.isEmpty()) {
            qCWarning(MIMETREEPARSER_LOG) << "unknown mimetype" << tMimeType;
        }
    }
    //WorkAround for #199083
    if (fileName == QLatin1String("text-vcard")) {
        fileName = QStringLiteral("text-x-vcard");
    }

    if (fileName.isEmpty()) {
        fileName = fallbackFileName1;
        if (fileName.isEmpty()) {
            fileName = fallbackFileName2;
        }
        if (!fileName.isEmpty()) {
            fileName = mimeDb.mimeTypeForFile(QLatin1String("/tmp/") + fileName).iconName();
        }
    }

    return fileName;
}

QString MimeTreeParser::Util::iconNameForContent(KMime::Content *node)
{
    if (!node) {
        return QString();
    }

    QByteArray mimeType = node->contentType()->mimeType();
    if (mimeType.isNull() || mimeType == "application/octet-stream") {
        const QString mime = mimetype(node->contentDisposition()->filename()).name();
        mimeType = mime.toLatin1();
    }
    mimeType = mimeType.toLower();
    return iconNameForMimetype(QLatin1String(mimeType), node->contentDisposition()->filename(),
                               node->contentType()->name());
}
