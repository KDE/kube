/*  -*- c++ -*-
    webkitparthtmlwriter.cpp

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

#include "webkitparthtmlwriter.h"
#include "messageviewer_debug.h"
#include "viewer/mailwebview.h"

#include <QUrl>

#include <cassert>
#include <QByteArray>
#include <QWebView>
#include <QWebPage>
#include <QWebFrame>
#include <QWebElement>

using namespace MessageViewer;

WebKitPartHtmlWriter::WebKitPartHtmlWriter(MailWebView *view, QObject *parent)
    : QObject(parent), HtmlWriter(),
      mHtmlView(view), mState(Ended)
{
    assert(view);
}

WebKitPartHtmlWriter::~WebKitPartHtmlWriter()
{
}

void WebKitPartHtmlWriter::begin(const QString &css)
{
    // The stylesheet is now included CSSHelper::htmlHead()
    Q_UNUSED(css);
    if (mState != Ended) {
        qCWarning(MESSAGEVIEWER_LOG) << "begin() called on non-ended session!";
        reset();
    }

    mEmbeddedPartMap.clear();

    // clear the widget:
    mHtmlView->setUpdatesEnabled(false);
    mHtmlView->scrollUp(10);
    // PENDING(marc) push into MailWebView
    mHtmlView->load(QUrl());
    mState = Begun;
}

void WebKitPartHtmlWriter::end()
{
    if (mState != Begun) {
        qCWarning(MESSAGEVIEWER_LOG) << "Called on non-begun or queued session!";
    }
    if (!mExtraHead.isEmpty()) {
        insertExtraHead();
        mExtraHead.clear();
    }
    mHtmlView->setHtml(mHtml, QUrl(QStringLiteral("file:///")));
    mHtmlView->show();
    mHtml.clear();

    resolveCidUrls();
    mHtmlView->scamCheck();
    mHtmlView->setUpdatesEnabled(true);
    mHtmlView->update();
    mState = Ended;
    Q_EMIT finished();
}

void WebKitPartHtmlWriter::reset()
{
    if (mState != Ended) {
        mHtml.clear();
        mState = Begun; // don't run into end()'s warning
        end();
        mState = Ended;
    }
}

void WebKitPartHtmlWriter::write(const QString &str)
{
    if (mState != Begun) {
        qCWarning(MESSAGEVIEWER_LOG) << "Called in Ended or Queued state!";
    }
    mHtml.append(str);
}

void WebKitPartHtmlWriter::queue(const QString &str)
{
    write(str);
}

void WebKitPartHtmlWriter::flush()
{
    mState = Begun; // don't run into end()'s warning
    end();
}

void WebKitPartHtmlWriter::embedPart(const QByteArray &contentId,
                                     const QString &contentURL)
{
    mEmbeddedPartMap[QLatin1String(contentId)] = contentURL;
}

void WebKitPartHtmlWriter::resolveCidUrls()
{
    // FIXME: instead of patching around in the HTML source, this should
    // be replaced by a custom QNetworkAccessManager (for QWebView), or
    // virtual loadResource() (for QTextBrowser)
    QWebElement root = mHtmlView->page()->mainFrame()->documentElement();
    QWebElementCollection images = root.findAll(QStringLiteral("img"));
    QWebElementCollection::iterator end(images.end());
    for (QWebElementCollection::iterator it = images.begin(); it != end; ++it) {
        QUrl url((*it).attribute(QStringLiteral("src")));
        if (url.scheme() == QLatin1String("cid")) {
            EmbeddedPartMap::const_iterator cit = mEmbeddedPartMap.constFind(url.path());
            if (cit != mEmbeddedPartMap.constEnd()) {
                qCDebug(MESSAGEVIEWER_LOG) << "Replacing" << url.toDisplayString() << "by" << cit.value();
                (*it).setAttribute(QStringLiteral("src"), cit.value());
            }
        }
    }
}

void WebKitPartHtmlWriter::insertExtraHead()
{
    const QString headTag(QStringLiteral("<head>"));
    const int index = mHtml.indexOf(headTag);
    if (index != -1) {
        mHtml.insert(index + headTag.length(), mExtraHead);
    }
}

void WebKitPartHtmlWriter::extraHead(const QString &str)
{
    mExtraHead = str;
}

