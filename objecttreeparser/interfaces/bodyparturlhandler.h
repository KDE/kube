/*  -*- c++ -*-
    interfaces/bodyparturlhandler.h

    This file is part of KMail's plugin interface.
    Copyright (c) 2003, 2004 Marc Mutz <mutz@kde.org>

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

#ifndef __KMAIL_INTERFACE_BODYPARTURLHANDLER_H__
#define __KMAIL_INTERFACE_BODYPARTURLHANDLER_H__

class QString;
class QPoint;

namespace MessageViewer
{

class Viewer;

namespace Interface
{

class BodyPart;

/**
  * @short An interface to body part reader link handlers
  * @author Marc Mutz <mutz@kde.org>
  *
  * This interface is a condensed of variant of the more general
  * @see URLHandler interface, designed to make bodypart-dependent
  * link operations possible without exposing KMail-internal
  * classes.
  *
  * Implementation-wise, these handlers are called as a nested
  * Chain Of Responsibilty by an internal implementation of
  * URLHandler.
  *
  * You can create a link whose handling is passed to this handler
  * by using BodyPart::makeLink( const QString & path ). \a path is
  * what * is passed back to the methods of this interface.
  *
  * Note that the BodyPart interface does not provide a means of
  * learning the content type of the body part passed. This is
  * intentional. It is expected that either separate
  * BodyPartURLHandlers are created for these purposes or else the
  * information encoded into the path parameter by the
  * BodyPartFormatter.
  */
class BodyPartURLHandler
{
public:
    virtual ~BodyPartURLHandler() {}

    /** Called when LMB-clicking on a link in the reader. Should
    start processing equivalent to "opening" the link.

    @return true if the click was handled by this handler, false
    otherwise.
    */
    virtual bool handleClick(Viewer *viewerInstance, BodyPart *part, const QString &path) const = 0;

    /** Called when RMB-clicking on a link in the reader. Should
    show a context menu at the specified point with the
    specified widget as parent.

    @return true if the right-click was handled by this handler,
    false otherwise.
    */
    virtual bool handleContextMenuRequest(BodyPart *part, const QString &path, const QPoint &p) const = 0;

    /** Called when hovering over a link.

        @return a string to be shown in the status bar while
    hovering over this link or QString() if the link was not
    handled by this handler.
    */
    virtual QString statusBarMessage(BodyPart *part, const QString &path) const = 0;
};

} // namespace Interface

}

#endif // __KMAIL_INTERFACES_BODYPARTURLHANDLER_H__

