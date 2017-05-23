/*
    Copyright (C) 2015 Sandro Knauß <knauss@kolabsys.com>
    Copyright (C) 2001,2002 the KPGP authors
    See file AUTHORS.kpgp for details

    Kmail is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "cryptohelper.h"

using namespace MimeTreeParser;

PGPBlockType Block::determineType() const
{
    const QByteArray data = text();
    if (data.startsWith("-----BEGIN PGP SIGNED")) {
        return ClearsignedBlock;
    } else if (data.startsWith("-----BEGIN PGP SIGNATURE")) {
        return SignatureBlock;
    } else if (data.startsWith("-----BEGIN PGP PUBLIC")) {
        return PublicKeyBlock;
    } else if (data.startsWith("-----BEGIN PGP PRIVATE")
               || data.startsWith("-----BEGIN PGP SECRET")) {
        return PrivateKeyBlock;
    } else if (data.startsWith("-----BEGIN PGP MESSAGE")) {
        if (data.startsWith("-----BEGIN PGP MESSAGE PART")) {
            return MultiPgpMessageBlock;
        } else {
            return PgpMessageBlock;
        }
    } else if (data.startsWith("-----BEGIN PGP ARMORED FILE")) {
        return PgpMessageBlock;
    } else if (data.startsWith("-----BEGIN PGP ")) {
        return UnknownBlock;
    } else {
        return NoPgpBlock;
    }
}

QList<Block> MimeTreeParser::prepareMessageForDecryption(const QByteArray &msg)
{
    PGPBlockType pgpBlock = NoPgpBlock;
    QList<Block>  blocks;
    int start = -1;   // start of the current PGP block
    int lastEnd = -1; // end of the last PGP block
    const int length = msg.length();

    if (msg.isEmpty()) {
        return blocks;
    }

    if (msg.startsWith("-----BEGIN PGP ")) {
        start = 0;
    } else {
        start = msg.indexOf("\n-----BEGIN PGP ") + 1;
        if (start == 0) {
            blocks.append(Block(msg, NoPgpBlock));
            return blocks;
        }
    }

    while (start != -1) {
        int nextEnd, nextStart;

        // is the PGP block a clearsigned block?
        if (!strncmp(msg.constData() + start + 15, "SIGNED", 6)) {
            pgpBlock = ClearsignedBlock;
        } else {
            pgpBlock = UnknownBlock;
        }

        nextEnd = msg.indexOf("\n-----END PGP ", start + 15);
        nextStart = msg.indexOf("\n-----BEGIN PGP ", start + 15);

        if (nextEnd == -1) {        // Missing END PGP line
            if (lastEnd != -1) {
                blocks.append(Block(msg.mid(lastEnd + 1), UnknownBlock));
            } else {
                blocks.append(Block(msg.mid(start), UnknownBlock));
            }
            break;
        }

        if ((nextStart == -1) || (nextEnd < nextStart) || (pgpBlock == ClearsignedBlock)) {
            // most likely we found a PGP block (but we don't check if it's valid)

            // store the preceding non-PGP block
            if (start - lastEnd - 1 > 0) {
                blocks.append(Block(msg.mid(lastEnd + 1, start - lastEnd - 1), NoPgpBlock));
            }

            lastEnd = msg.indexOf("\n", nextEnd + 14);
            if (lastEnd == -1) {
                if (start < length) {
                    blocks.append(Block(msg.mid(start)));
                }
                break;
            } else {
                blocks.append(Block(msg.mid(start, lastEnd + 1 - start)));
                if ((nextStart != -1) && (nextEnd > nextStart)) {
                    nextStart = msg.indexOf("\n-----BEGIN PGP ", lastEnd + 1);
                }
            }
        }

        start = nextStart;

        if (start == -1) {
            if (lastEnd + 1 < length) {
                //rest of mail is no PGP Block
                blocks.append(Block(msg.mid(lastEnd + 1), NoPgpBlock));
            }
            break;
        } else {
            start++; // move start behind the '\n'
        }
    }

    return blocks;
}

Block::Block(const QByteArray &m)
    : msg(m)
{
    mType = determineType();
}

Block::Block(const QByteArray &m, PGPBlockType t)
    : msg(m)
    , mType(t)
{

}

QByteArray MimeTreeParser::Block::text() const
{
    return msg;
}

PGPBlockType Block::type() const
{
    return mType;
}
