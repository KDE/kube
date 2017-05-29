/*
    cryptohelper.h

    Copyright (C) 2015 Sandro Knauß <knauss@kolabsys.com>
    Copyright (C) 2001,2002 the KPGP authors
    See file AUTHORS.kpgp for details

    KMail is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef __MIMETREEPARSER_CRYPTOHELPER_H__
#define __MIMETREEPARSER_CRYPTOHELPER_H__

#include <QByteArray>
#include <QList>

namespace MimeTreeParser
{

enum PGPBlockType {
    UnknownBlock = -1,        // BEGIN PGP ???
    NoPgpBlock = 0,
    PgpMessageBlock = 1,      // BEGIN PGP MESSAGE
    MultiPgpMessageBlock = 2, // BEGIN PGP MESSAGE, PART X[/Y]
    SignatureBlock = 3,       // BEGIN PGP SIGNATURE
    ClearsignedBlock = 4,     // BEGIN PGP SIGNED MESSAGE
    PublicKeyBlock = 5,       // BEGIN PGP PUBLIC KEY BLOCK
    PrivateKeyBlock = 6       // BEGIN PGP PRIVATE KEY BLOCK (PGP 2.x: ...SECRET...)
};

class Block
{
public:
    Block(const QByteArray &m);

    Block(const QByteArray &m, PGPBlockType t);

    QByteArray text() const;
    PGPBlockType type() const;
    PGPBlockType determineType() const;

    QByteArray msg;
    PGPBlockType mType;
};

/** Parses the given message and splits it into OpenPGP blocks and
    Non-OpenPGP blocks.
*/
QList<Block> prepareMessageForDecryption(const QByteArray &msg);

} // namespace MimeTreeParser

Q_DECLARE_TYPEINFO(MimeTreeParser::Block, Q_MOVABLE_TYPE);

#endif
