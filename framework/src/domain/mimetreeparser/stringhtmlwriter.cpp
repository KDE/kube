/*  -*- c++ -*-
    filehtmlwriter.cpp

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

#include "stringhtmlwriter.h"

#include <QDebug>
#include <QTextStream>
#include <QUrl>

StringHtmlWriter::StringHtmlWriter()
    : MimeTreeParser::HtmlWriter()
    , mState(Ended)
{
}

StringHtmlWriter::~StringHtmlWriter()
{
}

void StringHtmlWriter::begin(const QString &css)
{
    if (mState != Ended) {
        qWarning() << "begin() called on non-ended session!";
        reset();
    }

    mState = Begun;
    mExtraHead.clear();
    mHtml.clear();

    if (!css.isEmpty()) {
        write(QLatin1String("<!-- CSS Definitions \n") + css + QLatin1String("-->\n"));
    }
}

void StringHtmlWriter::end()
{
    if (mState != Begun) {
        qWarning() << "Called on non-begun or queued session!";
    }

    if (!mExtraHead.isEmpty()) {
        insertExtraHead();
        mExtraHead.clear();
    }
    resolveCidUrls();
    mState = Ended;
}

void StringHtmlWriter::reset()
{
    if (mState != Ended) {
        mHtml.clear();
        mExtraHead.clear();
        mState = Begun; // don't run into end()'s warning
        end();
        mState = Ended;
    }
}

void StringHtmlWriter::write(const QString &str)
{
    if (mState != Begun) {
        qWarning() << "Called in Ended or Queued state!";
    }
    mHtml.append(str);
}

void StringHtmlWriter::queue(const QString &str)
{
    write(str);
}

void StringHtmlWriter::flush()
{
    mState = Begun; // don't run into end()'s warning
    end();
}

void StringHtmlWriter::embedPart(const QByteArray &contentId, const QString &url)
{
    write("<!-- embedPart(contentID=" + contentId + ", url=" + url + ") -->\n");
    mEmbeddedPartMap.insert(contentId, url);
}

void StringHtmlWriter::resolveCidUrls()
{
    for (const auto &cid : mEmbeddedPartMap.keys()) {
        mHtml.replace(QString("src=\"cid:%1\"").arg(QString(cid)), QString("src=\"%1\"").arg(mEmbeddedPartMap.value(cid).toString()));
    }
}

void StringHtmlWriter::extraHead(const QString &extraHead)
{
    if (mState != Ended) {
        qWarning() << "Called on non-started session!";
    }
    mExtraHead.append(extraHead);
}


void StringHtmlWriter::insertExtraHead()
{
    const QString headTag(QStringLiteral("<head>"));
    const int index = mHtml.indexOf(headTag);
    if (index != -1) {
        mHtml.insert(index + headTag.length(), mExtraHead);
    }
}

QMap<QByteArray, QUrl> StringHtmlWriter::embeddedParts() const
{
    return mEmbeddedPartMap;
}

QString StringHtmlWriter::html() const
{
    if (mState != Ended) {
        qWarning() << "Called on non-ended session!";
    }
    return mHtml;
}
