/*  -*- c++ -*-
    interfaces/htmlwriter.h

    This file is part of KMail's plugin interface.
    Copyright (c) 2003 Marc Mutz <mutz@kde.org>

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

#ifndef __KMAIL_INTERFACES_HTMLWRITER_H__
#define __KMAIL_INTERFACES_HTMLWRITER_H__

class QByteArray;
class QString;

namespace MessageViewer
{
/**
  * @short An interface for HTML sinks.
  * @author Marc Mutz <mutz@kde.org>
  *
  */
namespace Interface
{
class HtmlWriter
{
public:
    virtual ~HtmlWriter() {}

    /** Signal the begin of stuff to write, and give the CSS definitions */
    virtual void begin(const QString &cssDefinitions) = 0;
    /** Write out a chunk of text. No HTML escaping is performed. */
    virtual void write(const QString &html) = 0;
    /** Signal the end of stuff to write. */
    virtual void end() = 0;
};
}

/**
  * @short An interface to HTML sinks
  * @author Marc Mutz <mutz@kde.org>
  *
  * @deprecated KMail should be ported to Interface::HtmlWriter. This
  * interface exposes internal working models. The queuing
  * vs. writing() issues exposed here should be hidden by using two
  * different implementations of KHTMLPartHtmlWriter: one for
  * queuing, and one for writing. This should be fixed before the
  * release, so we an keep the plugin interface stable.
  *
  * Operate this interface in one and only one of the following two
  * modes:
  *
  * @section Sync Mode
  *
  * In sync mode, use #begin() to initiate a session, then
  * #write() some chunks of HTML code and finally #end() the session.
  *
  * @section Async Mode
  *
  * In async mode, use #begin() to initialize a session, then
  * #queue() some chunks of HTML code and finally end the
  * session by calling #flush().
  *
  * Queued HTML code is fed to the html sink using a timer. For this
  * to work, control must return to the event loop so timer events
  * are delivered.
  *
  * @section Combined mode
  *
  * You may combine the two modes in the following way only. Any
  * number of #write() calls can precede #queue() calls,
  * but once a chunk has been queued, you @em must @em not
  * #write() more data, only #queue() it.
  *
  * Naturally, whenever you queued data in a given session, that
  * session must be ended by calling #flush(), not #end().
  */
class HtmlWriter : public Interface::HtmlWriter
{
public:
    virtual ~HtmlWriter() {}

    /** Stop all possibly pending processing in order to be able to
      *  call #begin() again. */
    virtual void reset() = 0;

    virtual void queue(const QString &str) = 0;
    /** (Start) flushing internal buffers, if any. */
    virtual void flush() = 0;

    /**
      * Embed a part with Content-ID @p contentId, using url @p url.
      */
    virtual void embedPart(const QByteArray &contentId, const QString &url) = 0;

    virtual void extraHead(const QString &str) = 0;
};

}

#endif // __KMAIL_INTERFACES_HTMLWRITER_H__

