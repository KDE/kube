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

#ifndef __MIMETREEPARSER_ENUMS_H__
#define __MIMETREEPARSER_ENUMS_H__

namespace MimeTreeParser
{

/**
 * The display update mode: Force updates the display immediately, Delayed updates
 * after some time (150ms by default)
 */
enum UpdateMode {
    Force = 0,
    Delayed
};

/** Flags for the encryption state. */
typedef enum {
    KMMsgEncryptionStateUnknown,
    KMMsgNotEncrypted,
    KMMsgPartiallyEncrypted,
    KMMsgFullyEncrypted,
    KMMsgEncryptionProblematic
} KMMsgEncryptionState;

/** Flags for the signature state. */
typedef enum {
    KMMsgSignatureStateUnknown,
    KMMsgNotSigned,
    KMMsgPartiallySigned,
    KMMsgFullySigned,
    KMMsgSignatureProblematic
} KMMsgSignatureState;

}

#endif
