/*  -*- c++ -*-
    webkitparthtmlwriter.h

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2003 Marc Mutz <mutz@kde.org>
    Copyright (c) 2009 Torgny Nyblom <nyblom@kde.org>

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

#ifndef MESSAGEVIEWER_WEBKITPARTHTMLWRITER_H
#define MESSAGEVIEWER_WEBKITPARTHTMLWRITER_H

#include "messageviewer/htmlwriter.h"
#include <QObject>

#include <QString>
#include <QByteArray>
#include <QMap>

namespace MessageViewer
{
class MailWebView;
}

namespace MessageViewer
{

class WebKitPartHtmlWriter : public QObject, public HtmlWriter
{
    Q_OBJECT
public:
    explicit WebKitPartHtmlWriter(MailWebView *view, QObject *parent = Q_NULLPTR);
    ~WebKitPartHtmlWriter();

    void begin(const QString &cssDefs) Q_DECL_OVERRIDE;
    void end() Q_DECL_OVERRIDE;
    void reset() Q_DECL_OVERRIDE;
    void write(const QString &str) Q_DECL_OVERRIDE;
    void queue(const QString &str) Q_DECL_OVERRIDE;
    void flush() Q_DECL_OVERRIDE;
    void embedPart(const QByteArray &contentId, const QString &url) Q_DECL_OVERRIDE;
    void extraHead(const QString &str) Q_DECL_OVERRIDE;
Q_SIGNALS:
    void finished();

private:
    void resolveCidUrls();
    void insertExtraHead();

private:
    MailWebView *mHtmlView;
    QString mHtml;
    QString mExtraHead;
    enum State {
        Begun,
        Queued,
        Ended
    } mState;
    // Key is Content-Id, value is URL
    typedef QMap<QString, QString> EmbeddedPartMap;
    EmbeddedPartMap mEmbeddedPartMap;
};

}

#endif // __MESSAGEVIEWER_WEBKITPARTHTMLWRITER_H__
