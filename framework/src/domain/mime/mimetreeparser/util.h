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

#ifndef __MIMETREEPARSER_UTILS_UTIL_H__
#define __MIMETREEPARSER_UTILS_UTIL_H__

#include <QString>

class QMimeType;

namespace KMime
{
class Content;
}

namespace MimeTreeParser
{

/**
 * The Util namespace contains a collection of helper functions use in
 * various places.
 */
namespace Util
{

/**
      * Describes the type of the displayed message. This depends on the MIME structure
      * of the mail and on whether HTML mode is enabled (which is decided by htmlMail())
      */
enum HtmlMode {
    Normal,         ///< A normal plaintext message, non-multipart
    Html,           ///< A HTML message, non-multipart
    MultipartPlain, ///< A multipart/alternative message, the plain text part is currently displayed
    MultipartHtml,  ///< A multipart/altervative message, the HTML part is currently displayed
    MultipartIcal   ///< A multipart/altervative message, the ICal part is currently displayed
};

bool isTypeBlacklisted(KMime::Content *node);

QString labelForContent(KMime::Content *node);

QMimeType mimetype(const QString &name);

QString iconNameForMimetype(const QString &mimeType,
        const QString &fallbackFileName1 = QString(),
        const QString &fallbackFileName2 = QString());

QString iconNameForContent(KMime::Content *node);
}
}

#endif
