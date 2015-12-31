/*  -*- c++ -*-
    teehtmlwriter.cpp

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

#include "teehtmlwriter.h"

namespace MessageViewer
{

TeeHtmlWriter::TeeHtmlWriter(HtmlWriter *writer1, HtmlWriter *writer2)
    : HtmlWriter()
{
    if (writer1) {
        mWriters.append(writer1);
    }
    if (writer2) {
        mWriters.append(writer2);
    }
}

TeeHtmlWriter::~TeeHtmlWriter()
{
    for (QList<HtmlWriter *>::Iterator it = mWriters.begin(); it != mWriters.end(); ++it) {
        delete(*it);
    }
}

void TeeHtmlWriter::addHtmlWriter(HtmlWriter *writer)
{
    if (writer) {
        mWriters.append(writer);
    }
}

void TeeHtmlWriter::begin(const QString &css)
{
    for (QList<HtmlWriter *>::Iterator it = mWriters.begin(); it != mWriters.end(); ++it) {
        (*it)->begin(css);
    }
}

void TeeHtmlWriter::end()
{
    for (QList<HtmlWriter *>::Iterator it = mWriters.begin(); it != mWriters.end(); ++it) {
        (*it)->end();
    }
}

void TeeHtmlWriter::reset()
{
    for (QList<HtmlWriter *>::Iterator it = mWriters.begin(); it != mWriters.end(); ++it) {
        (*it)->reset();
    }
}

void TeeHtmlWriter::write(const QString &str)
{
    for (QList<HtmlWriter *>::Iterator it = mWriters.begin(); it != mWriters.end(); ++it) {
        (*it)->write(str);
    }
}

void TeeHtmlWriter::queue(const QString &str)
{
    for (QList<HtmlWriter *>::Iterator it = mWriters.begin(); it != mWriters.end(); ++it) {
        (*it)->queue(str);
    }
}

void TeeHtmlWriter::flush()
{
    for (QList<HtmlWriter *>::Iterator it = mWriters.begin(); it != mWriters.end(); ++it) {
        (*it)->flush();
    }
}

void TeeHtmlWriter::embedPart(const QByteArray &contentId, const QString &url)
{
    for (QList<HtmlWriter *>::Iterator it = mWriters.begin(); it != mWriters.end(); ++it) {
        (*it)->embedPart(contentId, url);
    }
}
void TeeHtmlWriter::extraHead(const QString &) {}
} //
