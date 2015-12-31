/*  -*- c++ -*-
    teehtmlwriter.h

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

#ifndef __MESSAGEVIEWER_TEEHTMLWRITER_H__
#define __MESSAGEVIEWER_TEEHTMLWRITER_H__

#include "interfaces/htmlwriter.h"

#include <QList>

class QString;

namespace MessageViewer
{

/** @short A HtmlWriter that dispatches all calls to a list of other HtmlWriters
      @author Marc Mutz <mutz@kde.org>
  **/
class TeeHtmlWriter : public HtmlWriter
{
public:
    explicit TeeHtmlWriter(HtmlWriter *writer1 = 0,
                           HtmlWriter *writer2 = 0);
    virtual ~TeeHtmlWriter();

    void addHtmlWriter(HtmlWriter *writer);

    //
    // HtmlWriter Interface
    //
    void begin(const QString &cssDefs) Q_DECL_OVERRIDE;
    void end() Q_DECL_OVERRIDE;
    void reset() Q_DECL_OVERRIDE;
    void write(const QString &str) Q_DECL_OVERRIDE;
    void queue(const QString &str) Q_DECL_OVERRIDE;
    void flush() Q_DECL_OVERRIDE;
    void embedPart(const QByteArray &contentId, const QString &url) Q_DECL_OVERRIDE;
    void extraHead(const QString &str) Q_DECL_OVERRIDE;

private:
    /** We own the HtmlWriters added to us! */
    QList<HtmlWriter *> mWriters;
};

} // namespace MessageViewer

#endif // __MESSAGEVIEWER_TEEHTMLWRITER_H__
